// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

// codegen
void codegen(RObject exp, Environment env);
RcppExport SEXP rosa_codegen(SEXP expSEXP, SEXP envSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< RObject >::type exp(expSEXP);
    Rcpp::traits::input_parameter< Environment >::type env(envSEXP);
    codegen(exp, env);
    return R_NilValue;
END_RCPP
}
// PrintExp
void PrintExp(RObject x);
RcppExport SEXP rosa_PrintExp(SEXP xSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< RObject >::type x(xSEXP);
    PrintExp(x);
    return R_NilValue;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"rosa_codegen", (DL_FUNC) &rosa_codegen, 2},
    {"rosa_PrintExp", (DL_FUNC) &rosa_PrintExp, 1},
    {NULL, NULL, 0}
};

RcppExport void R_init_rosa(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}