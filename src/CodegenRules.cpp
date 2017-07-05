#include "CodegenRules.hpp"

#include <map>
#include <sstream>

namespace rosa {
namespace rule {
namespace codegen {

std::map<std::string, std::string> OpRule::names_ = {
    { "+", "plus" }, { "-", "minus" }, { "*", "times" }, { "/", "divide" },
    { "%%", "mod" }, { "^", "pow" }, { ">", "gt" }, { "<", "lt" },
    { ">=", "ge" }, { "<=", "le" }, { "==", "eq" },
    { "&&", "logic_and" }, { "||", "logic_or" }
};

std::map<std::string, std::string> OpRule::renames_ = {
    { "%%", "%" }, { "^", "_$NO_INFIX" }, { "/", "_$NO_INFIX" }
};

std::string OpRule::genCode(const std::vector<const Type*> &types,
                            const std::vector<std::string> &args) {
  std::ostringstream out;
  if (types.size() == 2) {
    const Type &tl = *types[0];
    const Type &tr = *types[1];
    const std::string &sl = args[0];
    const std::string &sr = args[1];

    if (tl.isScalar() && tr.isScalar() && op_ == "/" &&
        tl.getTypeID() != kDouble && tr.getTypeID() != kDouble) {
      out << "(" << sl << ")/((double)" << sr << ")";
    } else {
      const auto &rn = renames_.find(op_);
      if (rn == renames_.end()) {
        out << "(" << sl << ")" << op_ << "(" << sr << ")";
      } else if (rn->second == "_$NO_INFIX") {
        out << "rosa::" << names_[op_] << "(" << sl << ", "
            << sr << ", " << env_ << ")";
      } else {
        out << "(" << sl << ")" << rn->second << "(" << sr << ")";
      }
    }
  } else if (types.size() == 1) {
    // Unary op only for - +
    out << op_ << "(" << args[0] << ")";
  }
  return out.str();
}

std::string FirstArgRule::genCode(const std::vector<const Type*> &types,
                                  const std::vector<std::string> &args) {
  if (args.empty()) {
    return "";
  }
  return args[0];
}

std::string FnCallRule::genCode(const std::vector<const Type*> &types,
                                const std::vector<std::string> &args) {
  std::string code = fn_name_ + "(";
  for (const auto &arg : args) {
    code.append(arg + ", ");
  }
  return code + env_ + ")";
}

std::string BuiltinFnCallRule::genCode(const std::vector<const Type*> &types,
                                       const std::vector<std::string> &args) {
  std::string code = "rosa::" + fn_name_ + "(";
  for (const auto &arg : args) {
    code.append(arg + ", ");
  }
  return code + env_ + ")";
}

std::string ParenthesisRule::genCode(const std::vector<const Type*> &types,
                                     const std::vector<std::string> &args) {
  return "(" + args[0] + ")";
}

std::string BracketRule::genCode(const std::vector<const Type*> &types,
                                 const std::vector<std::string> &args) {
  return args[0] + "[(" + args[1] + ")-1]";
}

std::string ForCondRule::genCode(const std::vector<const Type*> &types,
                                 const std::vector<std::string> &args) {
  return args[0] + " : " + args[1];
}

}  // namespace codegen
}  // namespace rule
}  // namespace rosa
