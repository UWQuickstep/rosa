#ifndef ROSA_TEMPLATE_UTIL_HPP_
#define ROSA_TEMPLATE_UTIL_HPP_

#include <algorithm>
#include <memory>
#include <type_traits>
#include <vector>

#include "Ast.hpp"
#include "Macros.hpp"

#include <Rcpp.h>

namespace rosa {

template <int type_id>
struct SExpTypeTrait;

#define REGISTER_SEXP_TYPE(type_id, classname) \
template <> \
struct SExpTypeTrait<type_id> { \
  using type = classname; \
  static constexpr int kTypeID = type_id; \
};

REGISTER_SEXP_TYPE(NILSXP, NilSxp);
REGISTER_SEXP_TYPE(SYMSXP, SymSxp);
REGISTER_SEXP_TYPE(LISTSXP, ListSxp);
REGISTER_SEXP_TYPE(CLOSXP, CloSxp);
REGISTER_SEXP_TYPE(LANGSXP, LangSxp);
REGISTER_SEXP_TYPE(CHARSXP, CharSxp);
REGISTER_SEXP_TYPE(LGLSXP, LglSxp);
REGISTER_SEXP_TYPE(INTSXP, IntSxp);
REGISTER_SEXP_TYPE(REALSXP, RealSxp);
REGISTER_SEXP_TYPE(STRSXP, StrSxp);
REGISTER_SEXP_TYPE(VECSXP, VecSxp);

#undef REGISTER_SEXP_TYPE


template <int target_type_id>
inline auto Cast(const SPtr &sxp) {
  DCHECK(sxp->type() == target_type_id);
  return std::static_pointer_cast<
      const typename SExpTypeTrait<target_type_id>::type>(sxp);
}


template <typename Functor>
inline auto InvokeOnSPtr(const SPtr &sxp,
                         const Functor &functor) {
  switch (sxp->type()) {
    case NILSXP:
      return functor(Cast<NILSXP>(sxp));
    case SYMSXP:
      return functor(Cast<SYMSXP>(sxp));
    case LISTSXP:
      return functor(Cast<LISTSXP>(sxp));
    case CLOSXP:
      return functor(Cast<CLOSXP>(sxp));
    case LANGSXP:
      return functor(Cast<LANGSXP>(sxp));
    case CHARSXP:
      return functor(Cast<CHARSXP>(sxp));
    case LGLSXP:
      return functor(Cast<LGLSXP>(sxp));
    case INTSXP:
      return functor(Cast<INTSXP>(sxp));
    case REALSXP:
      return functor(Cast<REALSXP>(sxp));
    case STRSXP:
      return functor(Cast<STRSXP>(sxp));
    case VECSXP:
      return functor(Cast<VECSXP>(sxp));
    default:
      // Unhandled SEXP types.
      return functor(std::static_pointer_cast<const OtherSxp>(sxp));
  }
}


template <typename ContainerT, typename KeyT>
inline bool ContainsKey(
    const ContainerT &container, const KeyT& key,
    typename std::enable_if<
        !std::is_same<ContainerT, std::vector<KeyT>>::value>::type * = 0) {
  return container.find(key) != container.end();
}


template <typename ContainerT, typename KeyT>
inline bool ContainsKey(
    const ContainerT &container, const KeyT& key,
    typename std::enable_if<
        std::is_same<ContainerT, std::vector<KeyT>>::value>::type * = 0) {
  return std::find(container.begin(), container.end(), key) != container.end();
}

}  // namespace rosa

#endif  // ROSA_TEMPLATE_UTIL_HPP_
