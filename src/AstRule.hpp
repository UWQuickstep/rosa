#ifndef ROSA_AST_RULE_HPP_
#define ROSA_AST_RULE_HPP_

#include "Ast.hpp"
#include "Macros.hpp"

namespace rosa {

/**
 * @brief Base class for rules that analyze or transform R AST (RObject).
 */
class AstRule {
 public:
  /**
   * @brief Empty constructor.
   */
  AstRule() {}

  /**
   * @brief Virtual destructor.
   */
  virtual ~AstRule() {}

  /**
   * @brief Gets the rule name.
   *
   * @return The name of the rule.
   */
  virtual std::string getName() const = 0;

  /**
   * @brief Applies the rule to \p input.
   *
   * @param input The input expression AST.
   * @return The output AST after the rule is applied.
   */
  virtual SPtr apply(const SPtr &input) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(AstRule);
};

}  // namespace rosa

#endif  // ROSA_AST_RULE_HPP_
