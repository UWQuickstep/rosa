#' @useDynLib rosa
#' @importFrom Rcpp sourceCpp

library(Rcpp)

#' @export
codegen <- function(fun, fname = "", env = environment()) {
  if (fname == "") {
    fname = deparse(substitute(fun))
  }
  info <- codegen_impl(fun, fname, env)

  Sys.setenv('PKG_CXXFLAGS' = '-std=c++14')
  sourceCpp(code = info$code, env)
  Sys.unsetenv('PKG_CXXFLAGS')

  info
}
