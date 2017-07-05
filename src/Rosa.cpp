#include <signal.h>

#include "Ast.hpp"
#include "AstPrinter.hpp"
#include "AstRule.hpp"
#include "AstTransforms.hpp"
#include "Cfg.hpp"
#include "CfgAnalysis.hpp"
#include "Codegen.hpp"
#include "DebugUtil.hpp"
#include "Macros.hpp"
#include "ReachingDefinition.hpp"
#include "TemplateUtil.hpp"
#include "Type.hpp"
#include "TypeInference.hpp"
#include "VariableFacts.hpp"

#include <Rcpp.h>
using namespace Rcpp;

void handler(int sig) {
  FATAL_ERROR("segfault");
}

//' @export
// [[Rcpp::export]]
List codegen_impl(RObject exp, const std::string &fname, Environment env) {
  signal(SIGSEGV, handler);

  std::map<std::string, std::string> results;

  // TODO(jianqiao): Show proper error message.
  DCHECK(exp.sexp_type() == CLOSXP);

  rosa::SPtr ast = rosa::SExp::Create(exp);
  ast = rosa::LanguageToClosureTransform().apply(ast);
  ast = rosa::ControlTransform().apply(ast);
  ast = rosa::IfAssignTransform().apply(ast);

  rosa::VariableFacts vf(ast);
  rosa::TypeInference ti(ast, vf);
  rosa::Codegen cg(rosa::Cast<CLOSXP>(ast), ti, fname);

  return List::create(Named("ast") = rosa::AstPrinter<>::ToString(ast),
                      Named("analysis") = rosa::CfgPrinter::ToString(ast, vf),
                      Named("type") = rosa::TypePrinter::ToString(ti),
                      Named("code") = cg.getSourceCode());
}
