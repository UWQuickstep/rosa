#include <memory>

#include "Ast.hpp"
#include "Macros.hpp"

#include <Rcpp.h>

namespace rosa {

SExp::~SExp() {}

std::string SExp::type_name() const {
  switch (type_) {
    case NILSXP:     return "NILSXP";
    case SYMSXP:     return "SYMSXP";
    case RAWSXP:     return "RAWSXP";
    case LISTSXP:    return "LISTSXP";
    case CLOSXP:     return "CLOSXP";
    case ENVSXP:     return "ENVSXP";
    case PROMSXP:    return "PROMSXP";
    case LANGSXP:    return "LANGSXP";
    case SPECIALSXP: return "SPECIALSXP";
    case BUILTINSXP: return "BUILTINSXP";
    case CHARSXP:    return "CHARSXP";
    case LGLSXP:     return "LGLSXP";
    case INTSXP:     return "INTSXP";
    case REALSXP:    return "REALSXP";
    case CPLXSXP:    return "CPLXSXP";
    case STRSXP:     return "STRSXP";
    case DOTSXP:     return "DOTSXP";
    case ANYSXP:     return "ANYSXP";
    case VECSXP:     return "VECSXP";
    case EXPRSXP:    return "EXPRSXP";
    case BCODESXP:   return "BCODESXP";
    case EXTPTRSXP:  return "EXTPTRSXP";
    case WEAKREFSXP: return "WEAKREFSXP";
    case S4SXP:		   return "S4SXP";
    default:         return "<unknown>";
  }
}

SPtr SExp::Create(SEXP sxp) {
  switch (TYPEOF(sxp)) {
    case NILSXP:     return NilSxp::Create();
    case SYMSXP:     return SymSxp::Create(sxp);
    case LISTSXP:    return ListSxp::Create(sxp);
    case CLOSXP:     return CloSxp::Create(sxp);
    case LANGSXP:    return LangSxp::Create(sxp);
    case CHARSXP:    return CharSxp::Create(sxp);;
    case LGLSXP:     return LglSxp::Create(sxp);
    case INTSXP:     return IntSxp::Create(sxp);
    case REALSXP:    return RealSxp::Create(sxp);
    case STRSXP:     return StrSxp::Create(sxp);
    case VECSXP:     return VecSxp::Create(sxp);
    default:         return OtherSxp::Create(sxp);
  }
}


}  // namespace rosa
