#ifndef ROSA_DEBUG_UTIL_HPP_
#define ROSA_DEBUG_UTIL_HPP_

#include <sstream>
#include <string>

#include "Ast.hpp"
#include "AstRule.hpp"
#include "Cfg.hpp"
#include "Macros.hpp"
#include "TypeInference.hpp"
#include "VariableFacts.hpp"

namespace rosa {

class CfgPrinter {
public:
  static std::string ToString(const SPtr &input,
                              const VariableFacts &vf);

 private:
  static void Visit(const SPtr &sxp, const VariableFacts &vf, std::ostringstream &out);
  static void Visit(const CloPtr &sxp, const VariableFacts &vf, std::ostringstream &out);

  DISALLOW_COPY_AND_ASSIGN(CfgPrinter);
};

class TypePrinter {
 public:
  static std::string ToString(const TypeInference &ti);

 private:
  static void Visit(const SPtr &sxp,
                    const TypeInference &ti,
                    std::ostringstream &out);

  static void PrintSignature(const FnSignature &sig, std::ostringstream &out);
};

}  // namespace rosa

#endif  // ROSA_DEBUG_UTIL_HPP_
