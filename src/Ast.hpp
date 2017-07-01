#ifndef ROSA_AST_HPP_
#define ROSA_AST_HPP_

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "Macros.hpp"

#include <Rcpp.h>

namespace rosa {

class SExp;
class NilSxp;
class SymSxp;
class ListSxp;
class CloSxp;
class EnvSxp;
class LangSxp;
class CharSxp;
class LglSxp;
class IntSxp;
class RealSxp;
class StrSxp;
class VecSxp;
class OtherSxp;

typedef std::shared_ptr<const SExp> SPtr;
typedef std::shared_ptr<const NilSxp> NilPtr;
typedef std::shared_ptr<const SymSxp> SymPtr;
typedef std::shared_ptr<const ListSxp> ListPtr;
typedef std::shared_ptr<const CloSxp> CloPtr;
typedef std::shared_ptr<const EnvSxp> EnvPtr;
typedef std::shared_ptr<const LangSxp> LangPtr;
typedef std::shared_ptr<const CharSxp> CharPtr;
typedef std::shared_ptr<const LglSxp> LglPtr;
typedef std::shared_ptr<const IntSxp> IntPtr;
typedef std::shared_ptr<const RealSxp> RealPtr;
typedef std::shared_ptr<const StrSxp> StrPtr;
typedef std::shared_ptr<const VecSxp> VecPtr;
typedef std::shared_ptr<const OtherSxp> OtherPtr;

/**
 * @brief Base class for expression AST.
 */
class SExp {
 public:
  virtual ~SExp();

  inline int type() const {
    return type_;
  }

  std::string type_name() const;

  static SPtr Create(SEXP sxp);

 protected:
  SExp(const int type) : type_(type) {}

  int type_;

 private:
  DISALLOW_COPY_AND_ASSIGN(SExp);
};


/**
 * @brief Null SEXP.
 */
class NilSxp : public SExp {
 public:
  inline static NilPtr Create() {
    return NilPtr(new NilSxp());
  }

 private:
  NilSxp() : SExp(NILSXP) {}
};


/**
 * @brief Symbols.
 */
class SymSxp : public SExp {
 public:
  inline const std::string& name() const {
    return name_;
  }

  inline static SymPtr Create(SEXP sxp) {
    DCHECK(TYPEOF(sxp) == SYMSXP);
    return SymPtr(new SymSxp(CHAR(PRINTNAME(sxp))));
  }

  inline static SymPtr Create(const std::string &name) {
    return SymPtr(new SymSxp(name));
  }

 private:
  SymSxp(const std::string name)
      : SExp(SYMSXP), name_(name) {}

  std::string name_;
};


/**
 * @brief Lists of dotted pairs.
 */
class ListSxp : public SExp {
 public:
  inline const std::vector<SPtr>& tags() const {
    return tags_;
  }

  inline const SPtr& tag(const std::size_t i) const {
    return tags_[i];
  }

  inline const std::vector<SPtr>& values() const {
    return values_;
  }

  inline const SPtr& value(const std::size_t i) const {
    return values_[i];
  }

  inline std::size_t size() const {
    DCHECK(tags_.size() == values_.size());
    return values_.size();
  }

  inline static ListPtr Create(SEXP sxp) {
    DCHECK(TYPEOF(sxp) == LISTSXP);
    std::vector<SPtr> tags;
    std::vector<SPtr> values;
    for (SEXP lc = sxp; lc != R_NilValue; lc = CDR(lc)) {
      SEXP tag = TAG(lc);
      if (tag != nullptr) {
        tags.emplace_back(SExp::Create(tag));
      } else {
        tags.emplace_back(NilSxp::Create());
      }
      values.emplace_back(SExp::Create(CAR(lc)));
    }
    return ListPtr(new ListSxp(std::move(tags), std::move(values)));
  }

  template <typename TagT, typename ValueT>
  inline static ListPtr Create(TagT &&tags, ValueT &&values) {
    return ListPtr(new ListSxp(
        std::forward<TagT>(tags), std::forward<ValueT>(values)));
  }

  template <typename ValueT>
  inline static ListPtr CreateWithoutTags(ValueT &&values) {
    return ListPtr(new ListSxp(
      std::vector<SPtr>(values.size(), NilSxp::Create()),
      std::forward<ValueT>(values)));
  }

 private:
  template <typename TagT, typename ValueT>
  ListSxp(TagT &&tags, ValueT &&values)
      : SExp(LISTSXP),
        tags_(std::forward<TagT>(tags)),
        values_(std::forward<ValueT>(values)) {}

  std::vector<SPtr> tags_;
  std::vector<SPtr> values_;
};


/**
 * @brief Closures.
 */
class CloSxp : public SExp {
 public:
  inline const SPtr& formals() const {
    return formals_;
  }

  inline const SPtr& body() const {
    return body_;
  }

  inline const SPtr& environment() const {
    return environment_;
  }

  inline static CloPtr Create(SEXP sxp) {
    DCHECK(TYPEOF(sxp) == CLOSXP);
    return CloPtr(new CloSxp(SExp::Create(FORMALS(sxp)),
                             SExp::Create(BODY(sxp)),
                             SExp::Create(CLOENV(sxp))));
  }

  inline static CloPtr Create(
      const SPtr formals, const SPtr body, const SPtr environment) {
    return CloPtr(new CloSxp(formals, body, environment));
  }

 private:
  CloSxp(const SPtr formals, const SPtr body, const SPtr environment)
      : SExp(CLOSXP),
        formals_(formals),
        body_(body),
        environment_(environment) {}

  SPtr formals_;
  SPtr body_;
  SPtr environment_;
};


/**
 * @brief Language constructs.
 */
class LangSxp : public SExp {
 public:
  inline const SPtr& op() const {
    return op_;
  }

  inline const ListPtr& arguments() const {
    return arguments_;
  }

  inline static LangPtr Create(SEXP sxp) {
    DCHECK(TYPEOF(sxp) == LANGSXP);
    return LangPtr(new LangSxp(
        SExp::Create(CAR(sxp)),
        ListSxp::Create(CDR(sxp))));
  }

  inline static LangPtr Create(const SPtr op, const ListPtr arguments) {
    return LangPtr(new LangSxp(op, arguments));
  }

 private:
  LangSxp(const SPtr op, const ListPtr arguments)
      : SExp(LANGSXP), op_(op), arguments_(arguments) {}

  SPtr op_;
  ListPtr arguments_;
};


/**
 * @brief Single string.
 */
class CharSxp : public SExp {
 public:
  inline const std::string& value() const {
    return value_;
  }

  inline static CharPtr Create(SEXP sxp) {
    DCHECK(TYPEOF(sxp) == CHARSXP);
    return CharPtr(new CharSxp(CHAR(sxp)));
  }

  inline static CharPtr Create(const std::string value) {
    return CharPtr(new CharSxp(value));
  }

 private:
  CharSxp(const std::string &value)
      : SExp(CHARSXP), value_(value) {}

  std::string value_;
};


/**
 * @brief Logical vectors.
 */
class LglSxp : public SExp {
 public:
  inline const std::vector<bool>& values() const {
    return values_;
  }

  inline static LglPtr Create(SEXP sxp) {
    DCHECK(TYPEOF(sxp) == LGLSXP);
    const std::size_t n = XLENGTH(sxp);
    std::vector<bool> values;
    values.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      values.emplace_back(LOGICAL(sxp)[i]);
    }
    return LglPtr(new LglSxp(std::move(values)));
  }

  template <typename VectorT>
  inline static LglPtr Create(VectorT &&values) {
    return LglPtr(new LglSxp(std::forward<VectorT>(values)));
  }

 private:
  template <typename VectorT>
  LglSxp(VectorT &&values)
      : SExp(LGLSXP), values_(std::forward<VectorT>(values)) {}

  std::vector<bool> values_;
};


/**
 * @brief Integer vectors.
 */
class IntSxp : public SExp {
 public:
  inline const std::vector<int>& values() const {
    return values_;
  }

  inline static IntPtr Create(SEXP sxp) {
    DCHECK(TYPEOF(sxp) == INTSXP);
    const std::size_t n = XLENGTH(sxp);
    std::vector<int> values;
    values.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      values.emplace_back(INTEGER(sxp)[i]);
    }
    return IntPtr(new IntSxp(std::move(values)));
  }

  template <typename VectorT>
  inline static IntPtr Create(VectorT &&values) {
    return IntPtr(new IntSxp(std::forward<VectorT>(values)));
  }

 private:
  template <typename VectorT>
  IntSxp(VectorT &&values)
      : SExp(INTSXP), values_(std::forward<VectorT>(values)) {}

  std::vector<int> values_;
};


/**
 * @brief Numeric vectors.
 */
class RealSxp : public SExp {
 public:
  inline const std::vector<double>& values() const {
    return values_;
  }

  inline static RealPtr Create(SEXP sxp) {
    DCHECK(TYPEOF(sxp) == REALSXP);
    const std::size_t n = XLENGTH(sxp);
    std::vector<double> values;
    values.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      values.emplace_back(REAL(sxp)[i]);
    }
    return RealPtr(new RealSxp(std::move(values)));
  }

  template <typename VectorT>
  inline static RealPtr Create(VectorT &&values) {
    return RealPtr(new RealSxp(std::forward<VectorT>(values)));
  }

 private:
  template <typename VectorT>
  RealSxp(VectorT &&values)
      : SExp(REALSXP), values_(std::forward<VectorT>(values)) {}

  std::vector<double> values_;
};


/**
 * @brief String vectors.
 */
class StrSxp : public SExp {
 public:
  inline const std::vector<std::string>& values() const {
    return values_;
  }

  inline static StrPtr Create(SEXP sxp) {
    DCHECK(TYPEOF(sxp) == STRSXP);
    const std::size_t n = XLENGTH(sxp);
    std::vector<std::string> values;
    values.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      values.emplace_back(CHAR(STRING_ELT(sxp, i)));
    }
    return StrPtr(new StrSxp(std::move(values)));
  }

  template <typename VectorT>
  inline static StrPtr Create(VectorT &&values) {
    return StrPtr(new StrSxp(std::forward<VectorT>(values)));
  }

 private:
  template <typename VectorT>
  StrSxp(VectorT &&values)
      : SExp(STRSXP), values_(std::forward<VectorT>(values)) {}

  std::vector<std::string> values_;
};


/**
 * @brief Generic vectors.
 */
class VecSxp : public SExp {
 public:
  inline const std::vector<SPtr>& values() const {
    return values_;
  }

  inline static VecPtr Create(SEXP sxp) {
    DCHECK(TYPEOF(sxp) == VECSXP);
    const std::size_t n = XLENGTH(sxp);
    std::vector<SPtr> values;
    for (std::size_t i = 0; i < n; ++i) {
      values.emplace_back(SExp::Create(VECTOR_ELT(sxp, i)));
    }
    return VecPtr(new VecSxp(std::move(values)));
  }

  template <typename VectorT>
  inline static VecPtr Create(VectorT &&values) {
    return VecPtr(new VecSxp(std::forward<VectorT>(values)));
  }

 private:
  template <typename VectorT>
  VecSxp(VectorT &&values)
      : SExp(VECSXP), values_(std::forward<VectorT>(values)) {}

  std::vector<SPtr> values_;
};


/**
 * @brief Other expressions that we do not handle yet.
 */
class OtherSxp : public SExp {
 public:
  inline SEXP get() const {
    return sxp_;
  }

  inline static OtherPtr Create(SEXP sxp) {
    return OtherPtr(new OtherSxp(sxp));
  }

 private:
  OtherSxp(SEXP sxp)
      : SExp(TYPEOF(sxp)), sxp_(sxp) {}

  SEXP sxp_;
};

}  // namespace rosa

#endif  // ROSA_AST_HPP_
