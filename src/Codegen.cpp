#include "Codegen.hpp"

#include <sstream>
#include <string>

#include "Ast.hpp"
#include "AstPrinter.hpp"
#include "Cfg.hpp"
#include "CodegenRules.hpp"
#include "TemplateUtil.hpp"
#include "Type.hpp"
#include "TypeInference.hpp"

#include <Rcpp.h>

namespace rosa {

namespace {

template <typename T>
inline void PrintVector(std::ostringstream &out,
                        const std::vector<T> &vec,
                        const std::string &separator = ", ") {
  if (vec.empty()) {
    return;
  }
  out << vec.front();
  for (std::size_t i = 1; i < vec.size(); ++i) {
    out << separator << vec[i];
  }
}

}  // namespace

Codegen::Codegen(const CloPtr &ast,
                 const TypeInference &ti,
                 const std::string &fn_name)
    : ast_(ast), ti_(ti), catalog_(CfgCatalog::Instance()) {
  initCodegenRules();

  FnSignature empty_sig;
  fn_sigs_.emplace(empty_sig);
  indents_.emplace(0);

  genClosure(ast, fn_name, true /* top_level */);

  // Clean up.
  indents_.pop();
  fn_sigs_.pop();

  std::ostringstream out;
  out << lib_ << "\n";

  // Print declarations.
  for (const auto &decls : fn_decls_) {
    if (decls.first != fn_name) {
      for (const auto &decl : decls.second) {
        out << decl.second << "\n\n";
      }
    }
  }

  // Print the top level function definition.
  DCHECK(ContainsKey(fn_defs_, fn_name));
  const auto &top_level_def = fn_defs_.at(fn_name);
  DCHECK(top_level_def.size() == 1);
  out << "//' @export\n// [[Rcpp::export]]\n";
  out << top_level_def.begin()->second << "\n";

  // Print other function definitions.
  for (const auto &defs : fn_defs_) {
    if (defs.first != fn_name) {
      for (const auto &def : defs.second) {
        out << def.second << "\n\n";
      }
    }
  }

  source_code_ = out.str();
}

void Codegen::genCode(const SPtr &sxp, std::ostringstream &out) {
  switch (sxp->type()) {
    case NILSXP:
      return;
    case CLOSXP:
      return;
    case SYMSXP:
      genSym(Cast<SYMSXP>(sxp), out);
      return;
    case LANGSXP:
      genLang(Cast<LANGSXP>(sxp), out);
      return;
    case CHARSXP:
      out << Cast<CHARSXP>(sxp)->value();
      return;
    case LGLSXP:
      genNumLitVector<LGLSXP>(sxp, "bool", out);
      return;
    case INTSXP:
      genNumLitVector<INTSXP>(sxp, "int", out);
      return;
    case REALSXP:
      genNumLitVector<REALSXP>(sxp, "double", out);
      return;
    case STRSXP:
      genStrLitVector(Cast<STRSXP>(sxp), out);
      return;
    default:
      break;
  }
  FATAL_ERROR("Failed to codegen SEXP: " + ToCompactString(sxp));
}

std::string Codegen::genCode(const SPtr &sxp) {
  std::ostringstream oss;
  genCode(sxp, oss);
  return oss.str();
}


template <int sexp_type_id>
void Codegen::genNumLitVector(const SPtr &sxp,
                              const std::string &ctype,
                              std::ostringstream &out) {
  const auto &values = Cast<sexp_type_id>(sxp)->values();
  if (values.size() == 1) {
    out << values.front();
  } else {
    out << "std::vector<" << ctype << ">({";
    PrintVector(out, values);
    out << "})";
  }
}

void Codegen::genStrLitVector(const StrPtr &sxp, std::ostringstream &out) {
  const auto &values = sxp->values();
  if (values.size() == 1) {
    out << "\"" << values.front() << "\"";
  } else {
    out << "std::vector<std::string>({";
    bool first = true;
    for (const auto &value : values) {
      if (first) {
        first = false;
      } else {
        out << ", ";
      }
      out << "\"" << value << "\"";
    }
    out << "})";
  }
}

void Codegen::genSym(const SymPtr &sxp, std::ostringstream &out) {
  // TODO: Handle type coercion between declared type and "real type".
  out << sxp->name();
}

void Codegen::genLang(const LangPtr &sxp, std::ostringstream &out) {
  const IndentControl &ic = indents_.top();
  const auto &args = sxp->arguments()->values();
  const std::string &op_name = Cast<SYMSXP>(sxp->op())->name();

  // Handle user-defined functions.
  const auto udf = ti_.user_defined_fns().find(sxp);
  if (udf != ti_.user_defined_fns().end()) {
    const auto &fcf = udf->second.find(fn_sigs_.top());
    DCHECK(fcf != udf->second.end());
    const CloPtr &clo = fcf->second.first;
    const FnSignature &sig = fcf->second.second;
    auto &fns = fn_lookup_[clo];

    // Generate function definition.
    if (fns.find(sig) == fns.end()) {
      fn_sigs_.emplace(sig);
      indents_.emplace(0);

      genClosure(clo, st_.allocSymbol(op_name));

      indents_.pop();
      fn_sigs_.pop();
    }

    // Generate arguments.
    // TODO: type casting.
    std::vector<std::string> arg_sources;
    for (const auto &arg : args) {
      arg_sources.emplace_back(genCode(arg));
    }
    out << rule::codegen::FnCallRule(op_name, st_.env()).genCode({}, arg_sources);
    return;
  }

  // Handle control functions.
  if (op_name == "{") {
    for (const auto &stmt : sxp->arguments()->values()) {
      out << ExprToStatement(ic.spaces() + genCode(stmt));
    }
  } else if (op_name == "if") {
    out << "if (" << genCode(args[0]) << ") {\n";
    indents_.emplace(ic.inc(2));
    out << ExprToStatement(genCode(args[1]));
    if (args.size() >= 3) {
      out << ic.spaces() << "} else {\n";
      out << ExprToStatement(genCode(args[2]));
    }
    indents_.pop();
    out << ic.spaces() << "}\n";
  } else if (op_name == "while") {
    out << "while (" << genCode(args[0]) << ") {\n";
    indents_.emplace(ic.inc(2));
    out << ExprToStatement(genCode(args[1]));
    indents_.pop();
    out << ic.spaces() << "}\n";
  } else if (op_name == "for") {
    out << "for (";
    // Whether use c loop or foreach loop.
    const auto &cond_args = Cast<LANGSXP>(args[0])->arguments()->values();
    const SPtr &e = cond_args[1];
    LangPtr el;
    if (e->type() == LANGSXP &&
      Cast<SYMSXP>((el=Cast<LANGSXP>(e))->op())->name() == ":") {
      const std::string &var = Cast<SYMSXP>(cond_args[0])->name();
      out << var << " = " << genCode(el->arguments()->value(0)) << "; "
          << var << " <= " << genCode(el->arguments()->value(1)) << "; "
          << var << " += 1";
    } else {
      out << "auto &" << genCode(args[0]);
    }
    out << ") {\n";
    indents_.emplace(ic.inc(2));
    out << ExprToStatement(genCode(args[1]));
    indents_.pop();
    out << ic.spaces() << "}\n";
  } else if (op_name == "return") {
    if (args.empty()) {
      out << "return;";
    } else {
      out << "return " << genCode(args[0]) << ";";
    }
  } else {
    // Handle pre-defined functions.
    std::vector<const Type*> arg_types;
    std::vector<std::string> arg_sources;
    for (const auto &arg : args) {
      arg_types.emplace_back(&getType(arg));
      arg_sources.emplace_back(genCode(arg));
    }

    if (op_name == "=" || op_name == "<-") {
      if (arg_types[1]->getTypeID() != kFunction) {
        out << arg_sources[0] << " = " << arg_sources[1];
      }
    } else {
      const auto rule_it = code_gen_rules_.find(op_name);
      if (rule_it != code_gen_rules_.end()) {
        out << rule_it->second->genCode(arg_types, arg_sources);
      } else {
        // Treat the rest all as R calls.
        const Type &expected_result_type = getType(sxp);
        if (expected_result_type.isScalar() &&
            vec_scalar_ops_.find(op_name) != vec_scalar_ops_.end()) {
          out << "rosa::scalar<" << expected_result_type.getName() << ">"
              << "(" << genRCall(op_name, arg_sources)
              << ", " << st_.env() << ")";
        } else {
          out << genRCall(op_name, arg_sources);
        }
      }
    }
  }
}

std::string Codegen::genRCall(const std::string &fn_name,
                              const std::vector<std::string> &args) {
  std::ostringstream out;
  const auto it = op_renames_.find(fn_name);
  if (it == op_renames_.end()) {
    out << fn_name;
  } else {
    out << it->second;
  }
  out << "(";
  PrintVector(out, args);
  out << ")";
  r_fn_use_.top().emplace(fn_name);
  return out.str();
}

std::string Codegen::genType(const Type &type) {
  switch (type.getTypeID()) {
    case kNull:  // Fall through
    case kLogical:
    case kInteger:
    case kDouble:
    case kString:
      return type.getName();
    case kVector:  // Fall through
    case kMatrix: {
       const std::string postfix =
           type.getTypeID() == kVector ? "Vector" : "Matrix";
       const Type &param =
           static_cast<const ParametricType&>(type).getParameter();
       switch (param.getTypeID()) {
         case kLogical:
           return "Logical" + postfix;
         case kInteger:
           return "Integer" + postfix;
         case kDouble:
           return "Numeric" + postfix;
         case kString:
           return "String" + postfix;
         default:
           return "InvalidType";
       }
    }
    case kAny:
      return "RObject";
    default:
      return "InvalidType";
  }
}


void Codegen::genFormals(const ListPtr &formals,
                         const FnSignature &sig,
                         const bool top_level,
                         std::ostringstream &out) {
  const auto &args = formals->tags();
  for (std::size_t i = 0; i < args.size(); ++i) {
    const std::string &var = Cast<SYMSXP>(args[i])->name();
    out << genType(*sig.at(var)) << " " << var;
    if (i != args.size() - 1) {
      out << ", ";
    }
  }
  if (!top_level) {
    out << ", Environment " << st_.env();
  }
}

void Codegen::genClosure(const CloPtr &clo,
                         const std::string &name,
                         const bool top_level) {
  const auto &sig = fn_sigs_.top();
  fn_lookup_[clo][sig] = name;

  // Generate function head.
  std::ostringstream header_out;
  if (!top_level) {
    header_out << "inline ";
  }
  const Type &rtn_type = getRtnType(clo);
  if (rtn_type.getTypeID() != kFunction) {
    header_out << genType(rtn_type);
  }
  header_out << " " << name << "(";
  genFormals(clo->formals(), sig, top_level, header_out);
  header_out << ")";

  fn_decls_[name][sig] = header_out.str() + ";";

  // Generate variable declarations.
  std::ostringstream decl_out;
  if (top_level) {
    decl_out << "  Function env_fn(\"environment\", Environment::base_env());\n";
    decl_out << "  Environment env(env_fn());\n";
  }
  const auto &decl_types = ti_.decl_types().find(clo);
  DCHECK(decl_types != ti_.decl_types().end());
  for(const auto &vft : decl_types->second) {
    const std::string &var = vft.first;
    if (sig.find(var) == fn_sigs_.top().end()) {
      const auto &ft = vft.second.find(sig);
      if (ft != vft.second.end() && ft->second->getTypeID() != kFunction) {
        decl_out << "  " << genType(*ft->second) << " " << var << ";\n";
      }
    }
  }


  std::ostringstream body_out;
  r_fn_use_.emplace();

  // Generate body statements.
  indents_.emplace(2);
  genCode(clo->body(), body_out);
  indents_.pop();

  // Attach R function defs.
  for (const auto &fn : r_fn_use_.top()) {
    const auto it = op_renames_.find(fn);
    decl_out << "  " << "Function "
             << (it == op_renames_.end() ? fn : it->second)
             << "(\"" << fn << "\", env);\n";
  }
  r_fn_use_.pop();

  std::string code = header_out.str() + " {\n";
  if (!decl_out.str().empty()) {
    code.append(decl_out.str() + "\n");
  }
  code.append(body_out.str() + "}\n");
  fn_defs_[name][sig] = std::move(code);
}

const Type& Codegen::getType(const SPtr &sxp) {
  const auto sft = ti_.expr_types().find(sxp);
  if (sft != ti_.expr_types().end()) {
    const auto ft = sft->second.find(fn_sigs_.top());
    if (ft != sft->second.end()) {
      return *ft->second;
    }
  }
  FATAL_ERROR("Cannot get type for SEXP: " + ToCompactString(sxp));
}

const Type& Codegen::getRtnType(const CloPtr &clo) {
  const auto cft = ti_.rtn_types().find(clo);
  if (cft != ti_.rtn_types().end()) {
    const auto ft = cft->second.find(fn_sigs_.top());
    if (ft != cft->second.end()) {
      return *ft->second;
    }
  }
  FATAL_ERROR("Cannot get return type for CloSxp: " + ToCompactString(clo));
}

std::string Codegen::ExprToStatement(const std::string &code) {
  bool isBlank = true;
  for (const char c : code) {
    if (c != ' ' && c != '\n') {
      isBlank = false;
      break;
    }
  }
  if (isBlank) {
    return "";
  }

  const char c = code.back();
  if (c != '\n') {
    if (c == ';') {
      return code + "\n";
    } else {
      return code + ";\n";
    }
  } else {
    return code;
  }
}

void Codegen::initCodegenRules() {
  using namespace rule::codegen;

  op_renames_.emplace(":", st_.allocSymbol("series"));

  vec_scalar_ops_.emplace("rgamma");
  vec_scalar_ops_.emplace("rnorm");
  vec_scalar_ops_.emplace("sum");
  vec_scalar_ops_.emplace("mean");
  vec_scalar_ops_.emplace("runif");

  code_gen_rules_["("] = std::make_unique<ParenthesisRule>();
  code_gen_rules_["["] = std::make_unique<BracketRule>();
  code_gen_rules_["_$for_cond"] = std::make_unique<ForCondRule>();

  std::vector<std::string> binop_fns = {
      "+", "-", "*", "/", "^", "%%",
      ">", "<", ">=", "<=", "==", "&&", "||"
  };
  for (const auto &fn : binop_fns) {
    code_gen_rules_[fn] = std::make_unique<OpRule>(fn, st_.env());
  }

  std::vector<std::string> first_arg_fns = { "_$if_cond", "_$while_cond" };
  for (const auto &fn : first_arg_fns) {
    code_gen_rules_[fn] = std::make_unique<FirstArgRule>();
  }

  std::vector<std::string> builtin_fn_call_fns = {
      "sqrt", "floor", "length", "seq_along", "nrow", "ncol", ":"
  };
  for (const auto &fn : builtin_fn_call_fns) {
    const auto rename_it = op_renames_.find(fn);
    const std::string *fn_name =
        rename_it == op_renames_.end() ? &fn : &rename_it->second;
    code_gen_rules_[fn] = std::make_unique<BuiltinFnCallRule>(*fn_name, st_.env());
  }
}

#include "RosaLib.hpp"

// ROSA runtime library.
const std::string Codegen::lib_ = rosa_lib;

}  // namespace rosa
