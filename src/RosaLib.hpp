static const char *rosa_lib = R"V0G0N(
#include <cstddef>
#include <cmath>
#include <type_traits>

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::plugins(cpp14)]]

namespace rosa {

template <typename ElementT> struct RcppTypeTrait;
#define REGISTER_RCPP_TYPE(ET, VT, MT) \
  template <> struct RcppTypeTrait<ET> { \
    using vector = VT; \
    using matrix = MT; \
  };

REGISTER_RCPP_TYPE(bool, LogicalVector, LogicalMatrix);
REGISTER_RCPP_TYPE(int, IntegerVector, IntegerMatrix);
REGISTER_RCPP_TYPE(double, NumericVector, NumericMatrix);
REGISTER_RCPP_TYPE(std::string, StringVector, StringMatrix);

#undef REGISTER_RCPP_TYPE

template <typename LT, typename RT> struct TypeUniferTrait;
#define REGISTER_TYPE_UNIFIER(LT, RT, UT) \
  template <> struct TypeUniferTrait<LT, RT> { \
    using type = UT; \
  };

// TODO(jianqiao): Auto-gen the closure for the overall hierarchy.
REGISTER_TYPE_UNIFIER(int, int, int);
REGISTER_TYPE_UNIFIER(int, double, double);
REGISTER_TYPE_UNIFIER(double, int, double);
REGISTER_TYPE_UNIFIER(double, double, double);

#undef REGISTER_TYPE_UNIFIER


template <int RTYPE>
inline int length(const Rcpp::Vector<RTYPE> &vec, Environment env) {
  return vec.size();
}

inline double floor(const double value, Environment env) {
  return std::floor(value);
}

inline double sqrt(const double value, Environment env) {
  return std::sqrt(value);
}

inline double pow(const double base, const double exponent, Environment env) {
  return std::pow(base, exponent);
}

template <typename T, int RTYPE>
inline auto scalar(const Vector<RTYPE> &vec, Environment env) {
  return static_cast<T>(vec[0]);
}

template <typename T>
inline auto scalar(const RObject &obj, Environment env) {
  using VectorT = typename RcppTypeTrait<T>::vector;
  Function accessor("[", env);
  return VectorT(accessor(obj, 0))[0];
}

template <int RTYPE>
inline auto nrow(const Matrix<RTYPE> &mat, Environment env) {
  return mat.nrow();
}

template <int RTYPE>
inline auto ncol(const Matrix<RTYPE> &mat, Environment env) {
  return mat.ncol();
}

template <typename LT, typename RT>
inline auto series(const LT lhs, const RT rhs, Environment env) {
  using ElementT = decltype(lhs + rhs);
  using VectorT = typename RcppTypeTrait<ElementT>::vector;
  const std::size_t len = static_cast<std::size_t>(rhs - lhs);
  VectorT vec(len);
  for (std::size_t i = 0; i < len; ++i) {
    vec[i] = lhs + i;
  }
  return vec;
}

template <int RTYPE>
inline IntegerVector seq_along(const Vector<RTYPE> &vec, Environment env) {
  const int len = vec.size();
  IntegerVector seq(len);
  for (int i = 1; i <= len; ++i) {
    seq[len] = i;
  }
  return seq;
}

// TODO(jianqiao): Auto-gen the code for all operators and all combinations of
// argument types.
template <int L_RTYPE, int R_RTYPE,
          template <class> class StoragePolicy,
          int RHS_RTYPE, bool RHS_NA, typename RHS_T>
inline auto operator-(const Vector<L_RTYPE> &lhs,
                      const SubsetProxy<R_RTYPE, StoragePolicy, RHS_RTYPE,
                                        RHS_NA, RHS_T> &rhs) {
  using LT = typename std::remove_reference<typename Vector<L_RTYPE>::Proxy>::type;
  using RT = typename std::remove_reference<typename Vector<R_RTYPE>::Proxy>::type;
  using UT = typename RcppTypeTrait<typename TypeUniferTrait<LT, RT>::type>::vector;
  return static_cast<UT>(lhs) - static_cast<UT>(static_cast<SEXP>(rhs));
}

}  // namespace rosa
)V0G0N";
