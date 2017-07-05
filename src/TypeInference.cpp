#include "TypeInference.hpp"

#include <cmath>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <utility>

#include "Ast.hpp"
#include "Cfg.hpp"
#include "Macros.hpp"
#include "TemplateUtil.hpp"
#include "Type.hpp"

namespace rosa {

TypeInference::TypeInference(const SPtr &ast, const VariableFacts &vf)
    : ast_(ast), catalog_(CfgCatalog::Instance()), vf_(vf) {
  initTypeRules();

  FnSignature empty_sig;
  fn_sigs_.emplace(empty_sig);

  // TODO(jianqiao): Currently assumes that the topmost level is a closure
  // without argument.
  DCHECK(ast_->type() == CLOSXP);
  const CloPtr clo = Cast<CLOSXP>(ast_);
  rtn_types_[clo][empty_sig] = &inferFnCall(clo);
  fn_sigs_.pop();
}

const Type& TypeInference::infer(const SPtr &sxp) {
  const Type *t;
  switch (sxp->type()) {
    case NILSXP:
      t = &NilType::Instance();
      break;
    case SYMSXP:
      t = &inferSymbol(Cast<SYMSXP>(sxp));
      break;
    case CLOSXP:
      t = &FunType::Instance();
      break;
    case LANGSXP:
      t = &inferLang(Cast<LANGSXP>(sxp));
      break;
    case CHARSXP:
      t = &StrType::Instance();
      break;
    case LGLSXP:
      t = &scalarOrVector(LglType::Instance(),
                          Cast<LGLSXP>(sxp)->values().size());
      break;
    case INTSXP:
      t = &scalarOrVector(IntType::Instance(),
                          Cast<INTSXP>(sxp)->values().size());
      break;
    case REALSXP: {
      bool isInt = true;
      const auto &values = Cast<REALSXP>(sxp)->values();
      for (const double v : values) {
        double intpart;
        if (std::modf(v, &intpart) != 0.0) {
          isInt = false;
        }
      }
      if (isInt) {
        t = &scalarOrVector(IntType::Instance(), values.size());
      } else {
        t = &scalarOrVector(DblType::Instance(), values.size());
      }
      break;
    }
    case STRSXP:
      t = &VecType::Instance(StrType::Instance());
      break;
    default:
      t = &AnyType::Instance();
      break;
  }

  expr_types_[sxp][fn_sigs_.top()] = t;
  return *t;
}

const Type& TypeInference::inferSymbol(const SymPtr &sxp) {
  return inferVar(sxp->name(), sxp);
}

const Type& TypeInference::inferVar(const std::string &var, const SPtr &sxp) {
  if (var == "" || cfgs_.empty()) {
    return NilType::Instance();
  }

  const auto &mapping = cfgs_.top()->mapping();
  const auto &it_node = mapping.find(sxp);
  if (it_node == mapping.end()) {
    return NilType::Instance();
  }

  const Type *inferred_type = &NilType::Instance();
  const auto &facts = rds_.top()->getBefore(it_node->second);
  const auto &it_locs = facts->locations().find(var);
  if (it_locs != facts->locations().end()) {
    for (const auto &node : it_locs->second) {
      const Type *st = nullptr;
      if (node == cfgs_.top()->enterNode()) {
        // Look up function signature
        const auto &sig = fn_sigs_.top();
        const auto &pt = sig.find(var);
        if (pt != sig.end()) {
          st = pt->second;
        }
      } else {
        const auto &ts = def_types_[node][var];
        const auto it = ts.find(fn_sigs_.top());
        if (it != ts.end()) {
          st = it->second;
        }
      }
      if (st != nullptr) {
        inferred_type = &inferred_type->join(*st);
      }
    }
  }
  return *inferred_type;
}

const Type& TypeInference::inferLang(const LangPtr &sxp) {
  // Does not handle high order function calls
  if (sxp->op()->type() != SYMSXP) {
    return AnyType::Instance();
  }

  CfgPtr cfg = cfgs_.top();
  FnSignature sig = fn_sigs_.top();
  const auto &rd = rds_.top();
  const std::string &op_name = Cast<SYMSXP>(sxp->op())->name();

  // Handle user-defined functions
  if (isUserDefinedFunction(op_name, sxp)) {
    DCHECK(ContainsKey(cfg->mapping(), sxp));
    const auto &node = cfg->mapping().at(sxp);
    DCHECK(ContainsKey(rd->getBefore(node)->locations(), op_name));
    const auto &locs = rd->getBefore(node)->locations().at(op_name);
    // Only deals with single-assignment plain functions.
    if (locs.size() == 1) {
      DCHECK(ContainsKey(vf_.values(), (*locs.begin())->sp));
      const auto &values_at_sxp = vf_.values().at((*locs.begin())->sp);
      DCHECK(ContainsKey(values_at_sxp, op_name));
      const auto &values = values_at_sxp.at(op_name);
      if (values.size() == 1 && (*values.begin())->type() == CLOSXP) {
        const CloPtr &clo = Cast<CLOSXP>(*values.begin());
        FnSignature sig = inferSignature(clo->formals(), sxp->arguments());
        user_defined_fns_[sxp][fn_sigs_.top()] = std::make_pair(clo, sig);

        const Type *t = rtn_types_[clo][sig];
        if (t == nullptr) {
          fn_sigs_.push(sig);
          t = &inferFnCall(clo);
          fn_sigs_.pop();
          rtn_types_[clo][sig] = t;
        }
        return *t;
      }
    }
    return AnyType::Instance();
  }

  const auto &args = sxp->arguments();
  std::vector<const Type*> arg_types;
  for (const auto &arg : args->values()) {
    arg_types.push_back(&infer(arg));
  }

  // Handle pre-defined functions.
  auto const& fn_rule = type_rules_.find(op_name);
  if (fn_rule == type_rules_.end()) {
    return AnyType::Instance();
  }

  // TODO(jianqiao): Constant propagation to find values for args.
  const Type &inferred_type = fn_rule->second->apply(arg_types, args);
  if (op_name == "<-" || op_name == "=" || op_name == "_$for_cond") {
    const SPtr lp = sxp->arguments()->value(0);
    if (lp->type() == SYMSXP) {
      DCHECK(ContainsKey(cfg->mapping(), sxp));
      def_types_[cfg->mapping().at(sxp)]
                [Cast<SYMSXP>(lp)->name()]
                [fn_sigs_.top()] = &inferred_type;
    } else {
      constructSymType(lp, arg_types.front()->join(inferred_type));
    }
  }
  return inferred_type;
}

const Type& TypeInference::inferFnCall(const CloPtr &clo) {
  const CfgPtr cfg = catalog_.getCfg(clo);
  cfgs_.emplace(cfg);
  rds_.emplace(std::make_unique<ReachingDefinition>(cfg, clo, vf_));

  infer(clo->body());

  // Compute the "global" type for a variable in a function
  auto &decl_types = decl_types_[clo];
  for (const auto &node :  cfg->nodes()) {
    const auto &it = def_types_.find(node);
    if (it != def_types_.end()) {
      for (auto &ts : it->second) {
        const Type *&t = ts.second[fn_sigs_.top()];
        if (t == nullptr) {
          t = &AnyType::Instance();
        }
        const Type *dt = decl_types[ts.first][fn_sigs_.top()];
        if (dt == nullptr) {
          dt = &NilType::Instance();
        }
        decl_types[ts.first][fn_sigs_.top()] = &dt->join(*t);
      }
    }
  }
  for (const auto &it : fn_sigs_.top()) {
    const Type *dt = decl_types[it.first][fn_sigs_.top()];
    if (dt == nullptr) {
      dt = &NilType::Instance();
    }
    decl_types[it.first][fn_sigs_.top()] = &it.second->join(*dt);
  }

  // Infer return type.
  const Type *rt = &NilType::Instance();
  DCHECK(ContainsKey(cfg->pred(), cfg->exitNode()));
  const auto &preds = cfg->pred().at(cfg->exitNode());
  for (const auto &node : preds) {
    if (node->sp) {
      const Type *expr_type = expr_types_[node->sp][fn_sigs_.top()];
      if (expr_type != nullptr) {
        rt = &rt->join(*expr_type);
      }
    }
  }

  rds_.pop();
  cfgs_.pop();
  return *rt;
}

const Type& TypeInference::scalarOrVector(const Type &t, const std::size_t len) {
  if (len == 1) {
    return t;
  } else {
    return VecType::Instance(t);
  }
}

FnSignature TypeInference::inferSignature(
    const ListPtr &formals, const ListPtr &actuals) {
  FnSignature sig;

  std::vector<std::string> f_names;
  for (const auto &tag : formals->tags()) {
    f_names.emplace_back(Cast<SYMSXP>(tag)->name());
  }

  const auto &a_vars = actuals->values();
  const auto &a_tags = actuals->tags();

  // Map named parameters.
  for (std::size_t i = 0; i < a_vars.size(); i++) {
    if (a_tags[i]->type() == SYMSXP) {
      const std::string &name = Cast<SYMSXP>(a_tags[i])->name();
      DCHECK(ContainsKey(f_names, name));
      sig[name] = &infer(a_vars[i]);
    }
  }

  // Map positioned parameters.
  for (std::size_t i = 0; i < a_vars.size(); i++) {
    if (a_tags[i]->type() == NILSXP) {
      for (const auto &name : f_names) {
        if (sig.find(name) == sig.end()) {
          sig[name] = &infer(a_vars[i]);
          break;
        }
      }
    }
  }

  // Map default values.
  const auto &f_defaults = formals->values();
  for (std::size_t i = 0; i < f_defaults.size(); i++) {
    const auto &name = f_names[i];
    if (sig.find(name) == sig.end()) {
      sig[name] = &infer(f_defaults[i]);
    }
  }

  return sig;
}

void TypeInference::constructSymType(const SPtr &sxp, const Type &type) {
  const Type *t = &type;

  if (sxp->type() == SYMSXP) {
    DCHECK(ContainsKey(cfgs_.top()->mapping(), sxp));
    const CfgNodePtr cfg_node = cfgs_.top()->mapping().at(sxp);
    def_types_[cfg_node][Cast<SYMSXP>(sxp)->name()][fn_sigs_.top()] = t;
  } else if(sxp->type() == LANGSXP) {
    const LangPtr lang = Cast<LANGSXP>(sxp);
    const std::string &op_name = Cast<SYMSXP>(lang->op())->name();

    if (op_name == "[" || op_name == "[[") {
      const auto &args = lang->arguments()->values();
      // Some cases in 2-dimensional accessing.
      if (args.size() == 3) {
        if (op_name == "[[") {
          constructSymType(sxp, MatType::Instance(*t));
        } else {
          if (t->isScalar()) {
            constructSymType(args[0], MatType::Instance(*t));
          } else if (t->getTypeID() == kVector) {
            const Type &pt = static_cast<const VecType*>(t)->getParameter();
            constructSymType(args[0], MatType::Instance(pt));
          } else if (t->getTypeID() == kMatrix) {
            constructSymType(args[0], *t);
          }
        }
        return;
      }

      // Cases in 1-dimensional accessing.
      if (args.size() != 2) {
        return;
      }
      const Type *sym_type = &infer(args[0]);
      const Type *arg_type = &infer(args[1]);
      if (t->getTypeID() == kVector) {
        constructSymType(args[0], *t);
      } else if (arg_type->getTypeID() == kNull) {
        if (t->isScalar() && sym_type->getTypeID() == kVector) {
          t = &VecType::Instance(*t);
        } else if (sym_type->getTypeID() == kMatrix) {
          if (t->isScalar()) {
            t = &MatType::Instance(*t);
          } else if (t->getTypeID() == kVector) {
            t = &MatType::Instance(static_cast<const VecType*>(t)->getParameter());
          }
        }
        constructSymType(args[0], *t);
      } else if (sym_type->isScalar()) {
        constructSymType(args[0], VecType::Instance(*t));
      } else if (arg_type->isScalar() && sym_type->getTypeID() == kVector) {
        constructSymType(args[0], VecType::Instance(*t));
      } else if (arg_type->isScalar() && sym_type->getTypeID() == kMatrix) {
        constructSymType(args[0], MatType::Instance(*t));
      } else if (arg_type->getTypeID() == kVector) {
        constructSymType(args[0], *t);
      }
    }
  }
}

void TypeInference::initTypeRules() {
  using namespace rule::type;

  type_rules_["c"] = std::make_unique<ConcatRule>();
  type_rules_["{"] = std::make_unique<LastArgRule>();
  type_rules_["="] = std::make_unique<AssignmentRule>();
  type_rules_["["] = std::make_unique<SubsettingRule>();
  type_rules_[":"] = std::make_unique<GenSeriesRule>();
  type_rules_["<-"] = std::make_unique<AssignmentRule>();
  type_rules_["[["] = std::make_unique<SingleAccessRule>();
  type_rules_["if"] = std::make_unique<IfRule>();
  type_rules_["for"] = std::make_unique<StaticReturnRule>(AnyType::Instance());
  type_rules_["while"] = std::make_unique<StaticReturnRule>(AnyType::Instance());
  type_rules_["return"] = std::make_unique<ReturnRule>();
  type_rules_["vector"] = std::make_unique<CreateVectorRule>();
  type_rules_["matrix"] = std::make_unique<CreateMatrixRule>();

  type_rules_["_$if_cond"] = std::make_unique<FirstArgRule>();
  type_rules_["_$while_cond"] = std::make_unique<FirstArgRule>();
  type_rules_["_$for_cond"] = std::make_unique<ForCondRule>();

  type_rules_["as.logical"] = std::make_unique<AsScalarOrVectorRule>(
      LglType::Instance());
  type_rules_["as.integer"] = std::make_unique<AsScalarOrVectorRule>(
      IntType::Instance());
  type_rules_["as.double"] = std::make_unique<AsScalarOrVectorRule>(
      DblType::Instance());
  type_rules_["seq_along"] = std::make_unique<StaticReturnRule>(
      VecType::Instance(IntType::Instance()));

  // Simplified type rules here, no type-error checking.
  std::vector<std::string> rtn_first_arg_fns = { "abs" };
  for (const auto &name : rtn_first_arg_fns) {
    type_rules_[name] = std::make_unique<FirstArgRule>();
  }

  std::vector<std::string> join_fns = { "+", "-", "*", "^", "(", "%%" };
  for (const auto &name : join_fns) {
    type_rules_[name] = std::make_unique<JoinRule>();
  }

  std::vector<std::string> make_vec_fns = { "sample", "rep", "sort" };
  for (const auto &name : make_vec_fns) {
    type_rules_[name] = std::make_unique<MakeVecRule>();
  }

  std::vector<std::string> rtn_int_fns = { "length", "nrow", "ncol" };
  for (const auto &name : rtn_int_fns) {
    type_rules_[name] = std::make_unique<StaticReturnRule>(IntType::Instance());
  }

  std::vector<std::string> rtn_scalar_fns = { "sum", "mean" };
  for (const auto &name : rtn_scalar_fns) {
    type_rules_[name] = std::make_unique<ReturnScalarRule>();
  }

  std::vector<std::string> rtn_str_fns = { "paste" };
  for (const auto &name : rtn_str_fns) {
    type_rules_[name] = std::make_unique<StaticReturnRule>(StrType::Instance());
  }

  std::vector<std::string> rtn_adaptive_int_fns = { "floor" };
  for (const auto &name : rtn_adaptive_int_fns) {
    type_rules_[name] = std::make_unique<AdaptiveReturnRule>(IntType::Instance());
  }

  std::vector<std::string> rtn_adaptive_dbl_fns = { "sqrt", "/" };
  for (const auto &name : rtn_adaptive_dbl_fns) {
    type_rules_[name] = std::make_unique<AdaptiveReturnRule>(DblType::Instance());
  }

  // TODO: Fix arguments mapping problem.
  std::vector<std::string> rtn_dbl_vec_fns =
      { "numeric", "rgamma", "rnorm", "rbeta", "runif" };
  for (const auto &name : rtn_dbl_vec_fns) {
    type_rules_[name] = std::make_unique<AdaptiveScalarOrVectorOnFirstArgumentRule>(
        DblType::Instance());
  }

  std::vector<std::string> rtn_adaptive_join_fns = { ">", "<", ">=", "<=", "==" };
  for (const auto &name : rtn_adaptive_join_fns) {
    type_rules_[name] = std::make_unique<AdaptiveJoinReturnRule>(LglType::Instance());
  }
}

bool TypeInference::isUserDefinedFunction(
    const std::string &fn_name, const SPtr &sxp) {
  return inferVar(fn_name, sxp).getTypeID() == kFunction;
}

}  // namespace rosa
