#ifndef ROSA_TYPE_INFERENCE_HPP_
#define ROSA_TYPE_INFERENCE_HPP_

#include <map>
#include <stack>
#include <string>
#include <utility>

#include "Ast.hpp"
#include "Cfg.hpp"
#include "Macros.hpp"
#include "ReachingDefinition.hpp"
#include "Type.hpp"
#include "TypeRules.hpp"
#include "VariableFacts.hpp"

namespace rosa {

typedef std::map<std::string, const Type*> FnSignature;

class TypeInference {
 public:
  TypeInference(const SPtr &ast, const VariableFacts &vf);

  inline const std::map<CloPtr,
      std::map<std::string, std::map<FnSignature, const Type*>>>& decl_types() const {
    return decl_types_;
  }

  inline const std::map<CfgNodePtr,
      std::map<std::string, std::map<FnSignature, const Type*>>>& def_types() const {
    return def_types_;
  }

  inline const std::map<SPtr,
      std::map<FnSignature, const Type*>>& expr_types() const {
    return expr_types_;
  }

  inline const std::map<CloPtr,
      std::map<FnSignature, const Type*>>& rtn_types() const {
    return rtn_types_;
  }

  inline const std::map<LangPtr,
      std::map<FnSignature, std::pair<CloPtr, FnSignature>>>& user_defined_fns() const {
    return user_defined_fns_;
  }

  inline void printDefTypes() const {
    std::cerr << "---- ALL DEF TYPES ----\n";
    for (const auto &it1 : def_types_) {
      std::cerr << "[" << it1.first->id << "]\n";
      for (const auto &it2 : it1.second) {
        std::cerr << it2.first << ":";
        for (const auto &it3 : it2.second) {
          const Type *t = it3.second;
          if (t == nullptr) {
            std::cerr << " null";
          } else {
            std::cerr << " " << t->getName();
          }
        }
        std::cerr << "\n";
      }
    }
    std::cerr << "\n";
  }

protected:
  void initTypeRules();

  void constructSymType(const SPtr &sxp, const Type &type);

  const Type& infer(const SPtr &sxp);
  const Type& inferSymbol(const SymPtr &sxp);
  const Type& inferVar(const std::string& var, const SPtr &expr);
  const Type& inferLang(const LangPtr &sxp);
  const Type& inferFnCall(const CloPtr &sxp);

private:
  const Type& scalarOrVector(const Type &rtype, const std::size_t len);

  FnSignature inferSignature(const ListPtr &formals, const ListPtr &actuals);

  bool isUserDefinedFunction(const std::string &fn_name, const SPtr &p);

  std::map<CloPtr, std::map<std::string, std::map<FnSignature, const Type*>>> decl_types_;
  std::map<CfgNodePtr, std::map<std::string, std::map<FnSignature, const Type*>>> def_types_;
  std::map<SPtr, std::map<FnSignature, const Type*>> expr_types_;
  std::map<CloPtr, std::map<FnSignature, const Type*> > rtn_types_;
  std::map<LangPtr, std::map<FnSignature, std::pair<CloPtr, FnSignature>>> user_defined_fns_;

  SPtr ast_;
  CfgCatalog &catalog_;
  const VariableFacts &vf_;
  std::map<std::string, std::unique_ptr<rule::type::TypeRule>> type_rules_;

  std::stack<FnSignature> fn_sigs_;
  std::stack<CfgPtr> cfgs_;
  std::stack<std::unique_ptr<ReachingDefinition>> rds_;

  DISALLOW_COPY_AND_ASSIGN(TypeInference);
};


}  // namespace rosa

#endif  // ROSA_TYPE_INFERENCE_HPP_
