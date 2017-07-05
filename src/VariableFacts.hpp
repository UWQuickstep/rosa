#ifndef ROSA_VARIABLE_FACTS_HPP_
#define ROSA_VARIABLE_FACTS_HPP_

#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include "Ast.hpp"
#include "AstRule.hpp"
#include "Macros.hpp"

namespace rosa {

/**
 * @brief Rule that analyze the USE/DEF and constant value information of variables.
 */
class VariableFacts {
 public:
  VariableFacts(const SPtr &input);

  inline const std::map<SPtr, std::set<std::string>>& use() const {
    return use_;
  }

  inline const std::map<SPtr, std::set<std::string>>& def() const {
    return def_;
  }

  inline const std::map<SPtr, std::map<std::string, std::set<SPtr>>>&
      values() const {
    return values_;
  }

 private:
  void visit(const SPtr &input);

  void visit(const LangPtr &input);
  void visit(const ListPtr &input);
  void visit(const SymPtr &input);

  void merge(const SPtr &node, const std::vector<SPtr> &children);

  std::stack<bool> lvalue_mode_;
  std::map<SPtr, std::set<std::string>> use_;
  std::map<SPtr, std::set<std::string>> def_;
  std::map<SPtr, std::map<std::string, std::set<SPtr>>> values_;

  DISALLOW_COPY_AND_ASSIGN(VariableFacts);
};

}  // namespace rosa

#endif  // ROSA_VARIABLE_FACTS_HPP_
