#ifndef ROSA_AST_TRANSFORMS_HPP_
#define ROSA_AST_TRANSFORMS_HPP_

#include <string>

#include "Ast.hpp"
#include "AstRule.hpp"
#include "Macros.hpp"

namespace rosa {

class AstTransform : public AstRule {
 public:
  SPtr apply(const SPtr &input) override;

 protected:
  AstTransform() {}

  virtual SPtr applyInternal(const SPtr &sxp);

  virtual SPtr applyInternal(const CloPtr &sxp);
  virtual SPtr applyInternal(const LangPtr &sxp);
  virtual SPtr applyInternal(const ListPtr &sxp);

 private:
  DISALLOW_COPY_AND_ASSIGN(AstTransform);
};


/**
 * @brief Rule that transform a function definition into a closure.
 */
class LanguageToClosureTransform : public AstTransform {
 public:
  /**
   * @brief Constructor.
   */
  LanguageToClosureTransform() {}

  std::string getName() const override {
    return "LanguageToClosureTransform";
  }

 private:
  SPtr applyInternal(const LangPtr &sxp) override;

  DISALLOW_COPY_AND_ASSIGN(LanguageToClosureTransform);
};


/**
 * @brief Rule that transform the control structures (i.e. for, while, if) for
 *        easier processing.
 */
class ControlTransform : public AstTransform {
 public:
  /**
   * @brief Constructor.
   */
  ControlTransform() {}

  std::string getName() const override {
    return "ControlTransform";
  }

 private:
  SPtr applyInternal(const LangPtr &sxp) override;

  SPtr addBraces(const SPtr &sxp);

  DISALLOW_COPY_AND_ASSIGN(ControlTransform);
};


class IfAssignTransform : public AstTransform {
 public:
  /**
   * @brief Constructor.
   */
  IfAssignTransform() {}

  std::string getName() const override {
    return "IfAssignTransform";
  }

 private:
  SPtr applyInternal(const LangPtr &sxp) override;

  SPtr makeAssignment(const SPtr &sym, const SPtr &value);

  DISALLOW_COPY_AND_ASSIGN(IfAssignTransform);
};

}  // namespace rosa

#endif  // ROSA_AST_TRANSFORMS_HPP_
