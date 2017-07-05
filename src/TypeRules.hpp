#ifndef ROSA_TYPE_RULES_HPP_
#define ROSA_TYPE_RULES_HPP_

#include <cstddef>
#include <memory>
#include <vector>

#include "Ast.hpp"
#include "Macros.hpp"
#include "TemplateUtil.hpp"
#include "Type.hpp"

namespace rosa {
namespace rule {
namespace type {

class TypeRule {
 public:
  virtual ~TypeRule();

  virtual std::string getName() const = 0;

  virtual const Type& apply(const std::vector<const Type*> &args) const = 0;

  virtual const Type& apply(const std::vector<const Type*> &args,
                            const ListPtr &values) const;
};


class FirstArgRule : public TypeRule {
 public:
  FirstArgRule() {}

  std::string getName() const override {
    return "FirstArg";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    if (args.empty()) {
      return NilType::Instance();
    }
    return *args.front();
  }
};


class LastArgRule : public TypeRule {
 public:
  LastArgRule() {}

  std::string getName() const override {
    return "LastArg";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    if (args.empty()) {
      return NilType::Instance();
    }
    return *args.back();
  }
};


class AssignmentRule : public TypeRule {
 public:
  AssignmentRule() {}

  std::string getName() const override {
    return "Assignment";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    return *args[1];
  }
};


class ReturnRule : public TypeRule {
 public:
  ReturnRule() {}

  std::string getName() const override {
    return "Return";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    if (args.empty()) {
      return NilType::Instance();
    } else {
      return *args.front();
    }
  }
};


class ReturnScalarRule : public TypeRule {
 public:
  ReturnScalarRule() {}

  std::string getName() const override {
    return "ReturnScalar";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    if (args.size() > 0) {
      const Type *t = args[0];
      if (t->isScalar()) {
        return *t;
      } else if (t->getTypeID() == kVector) {
        return static_cast<const VecType*>(t)->getParameter();
      } else if (t->getTypeID() == kMatrix) {
        return static_cast<const MatType*>(t)->getParameter();
      }
    }
    return AnyType::Instance();
  }
};


class AsScalarOrVectorRule : public TypeRule {
 public:
  AsScalarOrVectorRule(const Type &as_type)
      : as_type_(as_type) {}

  std::string getName() const override {
    return "AsScalarOrVector";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    const Type *t;
    if (args.size() > 0) {
      t = args.front();
    } else {
      t = &NilType::Instance();
    }
    if (t->isScalar()) {
      return as_type_;
    }
    switch (t->getTypeID()) {
      case kVector:  // Fall through
      case kMatrix:
        return VecType::Instance(as_type_);
      default:
        break;
    }
    return AnyType::Instance();
  }

 private:
  const Type &as_type_;
};


class StaticReturnRule : public TypeRule {
 public:
  StaticReturnRule(const Type &rtn_type)
      : rtn_type_(rtn_type) {}

  std::string getName() const override {
    return "StaticReturn";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    return rtn_type_;
  }

 private:
  const Type &rtn_type_;
};


class AdaptiveReturnRule : public TypeRule {
 public:
  AdaptiveReturnRule(const Type &rtn_type)
      : rtn_type_(rtn_type) {}

  std::string getName() const override {
    return "AdaptiveReturn";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    const Type *t;
    if (args.size() > 0) {
      t = args.front();
    } else {
      t = &NilType::Instance();
    }
    if (t->isScalar()) {
      return rtn_type_;
    }
    switch (t->getTypeID()) {
      case kVector:
        return VecType::Instance(rtn_type_);
      case kMatrix:
        return MatType::Instance(rtn_type_);
      default:
        break;
    }
    return AnyType::Instance();
  }

 private:
  const Type &rtn_type_;
};


class AdaptiveScalarOrVectorOnFirstArgumentRule : public TypeRule {
 public:
  AdaptiveScalarOrVectorOnFirstArgumentRule(const Type &rtn_type)
      : rtn_type_(rtn_type) {}

  std::string getName() const override {
    return "AdaptiveScalarOrVectorOnFirstArgument";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    return VecType::Instance(rtn_type_);
  }

  const Type& apply(const std::vector<const Type*> &args,
                    const ListPtr &values) const override {
    if (values->size() == 0 || values->tag(0)->type() != NILSXP) {
      return apply(args);
    }

    const SPtr first_value = values->value(0);
    bool is_one = false;
    if (first_value->type() == INTSXP) {
      const auto &values = Cast<INTSXP>(first_value)->values();
      if (values.size() == 1 && values.front() == 1) {
        is_one = true;
      }
    } else if (first_value->type() == REALSXP) {
      const auto &values = Cast<REALSXP>(first_value)->values();
      if (values.size() == 1 && values.front() == 1) {
        is_one = true;
      }
    }

    if (is_one) {
      return rtn_type_;
    } else {
      return VecType::Instance(rtn_type_);
    }
  }

 private:
  const Type &rtn_type_;
};


class JoinRule : public TypeRule {
 public:
  JoinRule() {}

  std::string getName() const override {
    return "Join";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    const Type *t = &NilType::Instance();
    for (const auto &arg : args) {
      t = &t->join(*arg);
    }
    return *t;
  }
};


class IfRule : public TypeRule {
 public:
  IfRule() {}

  std::string getName() const override {
    return "If";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    const Type *t = &NilType::Instance();
    for (std::size_t i = 1; i < args.size(); ++i) {
      t = &t->join(*args[i]);
    }
    return *t;
  }
};


class GenSeriesRule : public TypeRule {
 public:
  GenSeriesRule() {}

  std::string getName() const override {
    return "GenSeries";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    const Type &t = args[0]->join(*args[1]);
    if (t.getTypeID() == kInteger || t.getTypeID() == kDouble) {
      return VecType::Instance(t);
    }
    // Should be type error
    return AnyType::Instance();
  }
};


class MakeVecRule : public TypeRule {
 public:
  MakeVecRule() {}

  std::string getName() const override {
    return "MakeVec";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    const Type *t;
    if (args.size() > 0) {
      t = args[0];
    } else {
      t = &NilType::Instance();
    }
    if (t->isScalar()) {
      return VecType::Instance(*t);
    } else if(t->getTypeID() == kMatrix) {
      return VecType::Instance(
          static_cast<const MatType*>(t)->getParameter());
    }
    return *t;
  }
};


class ConcatRule : public TypeRule {
 public:
  ConcatRule() {}

  std::string getName() const override {
    return "Concat";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    const Type &t = JoinRule().apply(args);
    if (t.isScalar()) {
      return VecType::Instance(t);
    } else if (t.getTypeID() == kMatrix) {
      return VecType::Instance(
          static_cast<const MatType&>(t).getParameter());
    }
    return t;
  }
};


class SubsettingRule : public TypeRule {
 public:
  SubsettingRule() {}

  std::string getName() const override {
    return "Subsetting";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    if (args.size() == 2) {
      // Cases in 1-dimensional subsetting
      const Type *h = args[0];
      const Type *a = args[1];
      if (h->getTypeID() == kMatrix) {
        std::vector<const Type*> nargs = args;
        nargs[0] = &VecType::Instance(
            static_cast<const MatType*>(h)->getParameter());
        return apply(nargs);
      } else if (a->getTypeID() == kNull) {
        return *h;
      } else if (a->getTypeID() == kInteger || a->getTypeID() == kDouble) {
        if (h->isScalar()) {
          return *h;
        } else if (h->getTypeID() == kVector) {
          return static_cast<const VecType*>(h)->getParameter();
        }
      } else if (a->getTypeID() == kVector) {
        return *h;
      }
    } else {
      // Some cases in 1-dimensional subsetting
      if (args[0]->getTypeID() == kMatrix) {
        const MatType *mat = static_cast<const MatType*>(args[0]);
        const Type *a1 = args[1];
        const Type *a2 = args[2];
        if (a1->isScalar() && a2->isScalar()) {
          return mat->getParameter();
        } else if ((a1->getTypeID() == kNull && a2->isScalar()) ||
            (a2->getTypeID() == kNull && a1->isScalar())) {
          return VecType::Instance(mat->getParameter());
        }
      }
    }
    return AnyType::Instance();
  }
};


class SingleAccessRule : public TypeRule {
 public:
  SingleAccessRule() {}

  std::string getName() const override {
    return "SingleAccess";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    const Type *h = args[0];
    if (args.size() == 2) {
      const Type *a = args[1];
      if (a->getTypeID() == kInteger || a->getTypeID() == kDouble) {
        if (h->isScalar()) {
          return *h;
        } else if (h->getTypeID() == kVector) {
          return static_cast<const VecType*>(h)->getParameter();
        }
      }
    } else if (args.size() == 3) {
      if (h->getTypeID() == kMatrix) {
        return static_cast<const MatType*>(h)->getParameter();
      }
    }
    return AnyType::Instance();
  }
};


class ForCondRule : public TypeRule {
 public:
  ForCondRule() {}

  std::string getName() const override {
    return "ForCond";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    const Type &t = MakeVecRule().apply({ args[1] });
    if (t.getTypeID() == kVector) {
      return static_cast<const VecType&>(t).getParameter();
    }
    return AnyType::Instance();
  }
};


class AdaptiveJoinReturnRule : public TypeRule {
 public:
  AdaptiveJoinReturnRule(const Type &rtn_type)
      : rtn_type_(rtn_type) {}

  std::string getName() const override {
    return "AdaptiveJoinReturn";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    const Type &t = JoinRule().apply(args);
    if (t.isScalar()) {
      return rtn_type_;
    } else if (t.getTypeID() == kVector) {
      return VecType::Instance(rtn_type_);
    } else if (t.getTypeID() == kMatrix) {
      return MatType::Instance(rtn_type_);
    }
    return AnyType::Instance();
  }

 private:
  const Type &rtn_type_;
};


class CreateMatrixRule : public TypeRule {
public:
 public:
  CreateMatrixRule() {}

  std::string getName() const override {
    return "CreateMatrix";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    return AnyType::Instance();
  }

  const Type& apply(const std::vector<const Type*> &args,
                    const ListPtr &values) const override {
    const int idx = values->indexOf(0, "data");
    const Type *t = idx >= 0 ? args[idx] : &LglType::Instance();
    if (t->isScalar()) {
      return MatType::Instance(*t);
    } else if (t->getTypeID() == kVector) {
      return MatType::Instance(
          static_cast<const VecType*>(t)->getParameter());
    } else if (t->getTypeID() == kMatrix) {
      return *t;
    }
    return AnyType::Instance();
  }
};


class CreateVectorRule : public TypeRule {
 public:
  CreateVectorRule() {}

  std::string getName() const override {
    return "CreateVector";
  }

  const Type& apply(const std::vector<const Type*> &args) const override {
    return AnyType::Instance();
  }

  const Type& apply(const std::vector<const Type*> &args,
                    const ListPtr &values) const override {
    SPtr mode = values->get(0, "mode");
    if (mode == nullptr) {
      return VecType::Instance(LglType::Instance());
    } else if (mode->type() == STRSXP) {
      const auto &strs = Cast<STRSXP>(mode)->values();
      if (strs.size() == 1) {
        const auto &str = strs.front();
        if (str == "logical") {
          return VecType::Instance(LglType::Instance());
        } else if (str == "integer") {
          return VecType::Instance(IntType::Instance());
        } else if (str == "numeric") {
          return VecType::Instance(DblType::Instance());
        } else if (str == "character") {
          return VecType::Instance(StrType::Instance());
        }
      }
    }

    return AnyType::Instance();
  }
};

}  // namespace type
}  // namespace rule
}  // namespace rosa

#endif  // ROSA_TYPE_RULES_HPP_
