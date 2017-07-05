#ifndef ROSA_CODEGEN_HPP_
#define ROSA_CODEGEN_HPP_

#include <cstddef>
#include <map>
#include <memory>
#include <stack>
#include <string>

#include "Cfg.hpp"
#include "CodegenRules.hpp"
#include "Macros.hpp"
#include "Type.hpp"
#include "TypeInference.hpp"

namespace rosa {

class SymbolTable {
 public:
  SymbolTable() {}

  inline std::string allocSymbol() {
    return allocSymbol("v");
  }

  inline std::string allocSymbol(const std::string &base) {
    std::string sym = base;
    std::size_t id = version_id_[base]++;
    if (id != 0) {
      sym.push_back('_');
      sym.append(std::to_string(id));
    }
    return sym;
  }

  inline std::string env() const {
    return "env";
  }

 private:
  std::map<std::string, std::size_t> version_id_;

  DISALLOW_COPY_AND_ASSIGN(SymbolTable);
};

class IndentControl {
 public:
  IndentControl(const std::size_t spaces = 0)
      : spaces_(spaces) {}

  inline std::string spaces() const {
    return std::string(spaces_, ' ');
  }

  inline IndentControl inc(const std::size_t delta) const {
    return IndentControl(spaces_ + delta);
  }

 private:
  std::size_t spaces_;
};

class Codegen;
typedef std::shared_ptr<Codegen> CodegenPtr;

class Codegen {
 public:
  Codegen(const CloPtr &ast, const TypeInference &ti, const std::string &fn_name);

  inline const std::string& getSourceCode() const {
    return source_code_;
  }

 private:
  void initCodegenRules();

  void genCode(const SPtr &sxp, std::ostringstream &out);
  std::string genCode(const SPtr &sxp);

  template <int sexp_type_id>
  void genNumLitVector(const SPtr &sxp,
                       const std::string &ctype,
                       std::ostringstream &out);

  void genStrLitVector(const StrPtr &sxp, std::ostringstream &out);

  void genSym(const SymPtr &sxp, std::ostringstream &out);
  void genLang(const LangPtr &sxp, std::ostringstream &out);

  std::string genRCall(const std::string &fn_name,
                       const std::vector<std::string> &args);

  void genFormals(const ListPtr &formals,
                  const FnSignature &sig,
                  const bool top_level,
                  std::ostringstream &out);
  void genClosure(const CloPtr &clo,
                  const std::string &fn_name,
                  const bool top_level = false);

  std::string genType(const Type &type);
  const Type& getType(const SPtr &sxp);
  const Type& getRtnType(const CloPtr &sxp);

  static std::string ExprToStatement(const std::string &code);

  const SPtr ast_;
  const TypeInference &ti_;
  CfgCatalog &catalog_;
  SymbolTable st_;

  std::string source_code_;

  std::stack<FnSignature> fn_sigs_;
  std::stack<IndentControl> indents_;

  std::map<std::string, std::map<FnSignature, std::string>> fn_decls_;
  std::map<std::string, std::map<FnSignature, std::string>> fn_defs_;
  std::map<CloPtr, std::map<FnSignature, std::string>> fn_lookup_;
  std::stack<std::set<std::string>> r_fn_use_;

  std::map<std::string, std::unique_ptr<rule::codegen::CodegenRule>> code_gen_rules_;
  std::map<std::string, std::string> op_renames_;
  std::set<std::string> vec_scalar_ops_;

  static const std::string lib_;

  DISALLOW_COPY_AND_ASSIGN(Codegen);
};

}

#endif  // ROSA_CODEGEN_HPP_
