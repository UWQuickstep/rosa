#' @useDynLib rosa
#' @importFrom Rcpp sourceCpp

library(Rcpp)

#' @export
codegen <- function(fun, fname = "", env = environment()) {
  if (fname == "") {
    fname = deparse(substitute(fun))
  }
  info <- codegen_impl(fun, fname, env)
  sourceCpp(code = info$code, env)
  info
}
