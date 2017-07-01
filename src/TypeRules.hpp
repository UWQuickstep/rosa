#ifndef ROSA_TYPE_RULES_HPP_
#define ROSA_TYPE_RULES_HPP_

#include <cstddef>
#include <memory>
#include <vector>

#include "Type.hpp"

namespace rosa {

class TypeRule {
 public:
  virtual ~TypeRule();
  virtual std::string getName() const = 0;
  virtual const Type& apply(const std::vector<const Type*> &args) = 0;
};


class FirstArgRule : public TypeRule {
 public:
  FirstArgRule() {}

  std::string getName() const override {
    return "FirstArg";
  }

  const Type& apply(const std::vector<const Type*> &args) override {
    return *args.front();
  }
};


class LastArgRule : public TypeRule {
 public:
  LastArgRule() {}

  std::string getName() const override {
    return "LastArg";
  }

  const Type& apply(const std::vector<const Type*> &args) override {
    return *args.back();
  }
};


class AssignmentRule : public TypeRule {
 public:
  AssignmentRule() {}

  std::string getName() const override {
    return "Assignment";
  }

  const Type& apply(const std::vector<const Type*> &args) override {
    return *args[1];
  }
};


class AsScalarRule : public TypeRule {
 public:
  AsScalarRule(const Type &as_type)
      : as_type_(as_type) {}

  std::string getName() const override {
    return "AsScalar";
  }

  const Type& apply(const std::vector<const Type*> &args) override {
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

  const Type& apply(const std::vector<const Type*> &args) override {
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

  const Type& apply(const std::vector<const Type*> &args) override {
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


class JoinRule : public TypeRule {
 public:
  JoinRule() {}

  std::string getName() const override {
    return "Join";
  }

  const Type& apply(const std::vector<const Type*> &args) override {
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

  const Type& apply(const std::vector<const Type*> &args) override {
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

  const Type& apply(const std::vector<const Type*> &args) override {
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

  const Type& apply(const std::vector<const Type*> &args) override {
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

  const Type& apply(const std::vector<const Type*> &args) override {
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


class MatrixRule : public TypeRule {
public:
 public:
  MatrixRule() {}

  std::string getName() const override {
    return "Matrix";
  }

  const Type& apply(const std::vector<const Type*> &args) override {
    if (args.size() > 0) {
      const Type *t = args[0];
      if (t->isScalar()) {
        return MatType::Instance(*t);
      } else if (t->getTypeID() == kVector) {
        return MatType::Instance(
            static_cast<const VecType*>(t)->getParameter());
      } else if (t->getTypeID() == kMatrix) {
        return *t;
      }
    }
    return AnyType::Instance();
  }
};


class SubsettingRule : public TypeRule {
 public:
  SubsettingRule() {}

  std::string getName() const override {
    return "Subsetting";
  }

  const Type& apply(const std::vector<const Type*> &args) override {
    // Currently only deals with 1-dimensional subsetting
    if (args.size() == 2) {
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

  const Type& apply(const std::vector<const Type*> &args) override {
    // Currently only deals with 1-dimensional access
    if (args.size() == 2) {
      const Type *h = args[0];
      const Type *a = args[1];
      if (a->getTypeID() == kInteger || a->getTypeID() == kDouble) {
        if (h->isScalar()) {
          return *h;
        } else if (h->getTypeID() == kVector) {
          return static_cast<const VecType*>(h)->getParameter();
        }
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

  const Type& apply(const std::vector<const Type*> &args) override {
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

  const Type& apply(const std::vector<const Type*> &args) override {
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

}  // namespace rosa

#endif  // ROSA_TYPE_RULES_HPP_
