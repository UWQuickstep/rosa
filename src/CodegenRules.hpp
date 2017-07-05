#ifndef ROSA_CODEGEN_RULES_HPP_
#define ROSA_CODEGEN_RULES_HPP_

#include <vector>
#include <map>
#include <memory>
#include <string>

#include "Macros.hpp"
#include "Type.hpp"

namespace rosa {
namespace rule {
namespace codegen {

class CodegenRule {
 public:
  CodegenRule() {}

  virtual ~CodegenRule() {}

  virtual std::string genCode(const std::vector<const Type*> &types,
                              const std::vector<std::string> &args) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(CodegenRule);
};

class OpRule : public CodegenRule {
 public:
  OpRule(const std::string &op, const std::string &env)
      : op_(op), env_(env) {}

  std::string genCode(const std::vector<const Type*> &types,
                      const std::vector<std::string> &args) override;

 private:
  const std::string op_;
  const std::string env_;
  static std::map<std::string, std::string> names_;
  static std::map<std::string, std::string> renames_;

  DISALLOW_COPY_AND_ASSIGN(OpRule);
};

class FirstArgRule : public CodegenRule {
 public:
  FirstArgRule() {}

  std::string genCode(const std::vector<const Type*> &types,
                      const std::vector<std::string> &args) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(FirstArgRule);
};

class FnCallRule : public CodegenRule {
 public:
  FnCallRule(const std::string &fn_name, const std::string &env)
      : fn_name_(fn_name), env_(env) {}

  std::string genCode(const std::vector<const Type*> &types,
                      const std::vector<std::string> &args) override;

 private:
  const std::string fn_name_;
  const std::string env_;

  DISALLOW_COPY_AND_ASSIGN(FnCallRule);
};

class BuiltinFnCallRule : public CodegenRule {
 public:
  BuiltinFnCallRule(const std::string &fn_name,
                    const std::string &env)
      : fn_name_(fn_name), env_(env) {}

  std::string genCode(const std::vector<const Type*> &types,
                      const std::vector<std::string> &args) override;

 private:
  const std::string fn_name_;
  const std::string env_;

  DISALLOW_COPY_AND_ASSIGN(BuiltinFnCallRule);
};

class ParenthesisRule : public CodegenRule {
 public:
  ParenthesisRule() {}

  std::string genCode(const std::vector<const Type*> &types,
                      const std::vector<std::string> &args) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ParenthesisRule);
};

class BracketRule : public CodegenRule {
 public:
  BracketRule() {}

  std::string genCode(const std::vector<const Type*> &types,
                      const std::vector<std::string> &args) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(BracketRule);
};

class ForCondRule : public CodegenRule {
public:
  ForCondRule() {}

  std::string genCode(const std::vector<const Type*> &types,
                      const std::vector<std::string> &args) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ForCondRule);
};

}  // namespace codegen
}  // namespace rule
}  // namespace rosa

#endif  // ROSA_CODEGEN_RULES_HPP_
