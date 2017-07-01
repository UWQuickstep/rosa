#include "Ast.hpp"
#include "AstPrinter.hpp"
#include "AstRule.hpp"
#include "AstTransforms.hpp"
#include "Cfg.hpp"
#include "CfgAnalysis.hpp"
#include "DebugUtil.hpp"
#include "ReachingDefinition.hpp"
#include "Type.hpp"
#include "TypeInference.hpp"
#include "VariableFacts.hpp"

#include <Rcpp.h>
using namespace Rcpp;

//' @export
// [[Rcpp::export]]
void codegen(RObject exp, Environment env) {
  rosa::SPtr ast = rosa::SExp::Create(exp);
  ast = rosa::LanguageToClosureTransform().apply(ast);
  ast = rosa::ControlTransform().apply(ast);
  ast = rosa::IfAssignTransform().apply(ast);

  Rcout << rosa::AstPrinter<>::ToString(ast) << "\n";
  Rcout << "\n--------\n--------\n--------\n";
  Rcout << rosa::AstPrinter<rosa::AstPrintFormat::kCompact>::ToString(ast) << "\n";
  Rcout << "\n--------\n--------\n--------\n";

  rosa::VariableFacts vf(ast);
  Rcout << rosa::CfgPrinter::ToString(ast, vf);

  rosa::TypeInference ti(ast, vf);
  Rcout << rosa::TypePrinter::ToString(ti);
}
