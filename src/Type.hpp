#ifndef ROSA_TYPE_HPP_
#define ROSA_TYPE_HPP_

#include <cstddef>
#include <string>
#include <unordered_map>

#include "HashUtil.hpp"
#include "Macros.hpp"

namespace rosa {

enum TypeID {
  kNull = 0,
  kLogical,
  kInteger,
  kDouble,
  kString,
  kVector,
  kMatrix,
  kFunction,
  kAny
};

class Type {
 public:
  virtual ~Type() {}

  inline TypeID getTypeID() const {
    return type_id_;
  }

  virtual std::string getName() const = 0;

  virtual bool equals(const Type &other) const = 0;

  virtual std::size_t hash() const = 0;

  virtual bool isScalar() const;

  virtual bool isParametric() const;

  static const Type& JoinType(const Type &lhs, const Type &rhs);

  inline const Type& join(const Type &other) const {
    return JoinType(*this, other);
  }

 protected:
  Type(const TypeID type_id)
      : type_id_(type_id) {}

 private:
  const TypeID type_id_;

  DISALLOW_COPY_AND_ASSIGN(Type);
};


struct TypeHasher {
  inline std::size_t operator()(const Type *type) const {
    return type->hash();
  };
};


template <typename T>
class PlainTypeInstancePolicy {
 public:
  static const T& Instance() {
    static T instance;
    return instance;
  }
};


template <typename T>
class ParametricTypeInstancePolicy {
 public:
  static const T& Instance(const Type &parameter) {
    static std::unordered_map<const Type*,
                              std::unique_ptr<const T>,
                              TypeHasher> instance_map;
    auto imit = instance_map.find(&parameter);
    if (imit != instance_map.end()) {
      return *imit->second;
    } else {
      const T *instance = new T(parameter);
      instance_map.emplace(&parameter, std::unique_ptr<const T>(instance));
      return *instance;
    }
  }
};


class ScalarType : public Type {
 public:
  bool equals(const Type &other) const override {
    return getTypeID() == other.getTypeID();
  }

  std::size_t hash() const override {
    return getTypeID();
  }

  bool isScalar() const override {
    return true;
  }

 protected:
  ScalarType(const TypeID type_id)
      : Type(type_id) {}

 private:
  DISALLOW_COPY_AND_ASSIGN(ScalarType);
};


class NilType : public ScalarType,
                public PlainTypeInstancePolicy<NilType> {
 public:
  std::string getName() const override {
    return "void";
  }

 private:
  NilType() : ScalarType(kNull) {}

  friend class PlainTypeInstancePolicy<NilType>;

  DISALLOW_COPY_AND_ASSIGN(NilType);
};


class LglType : public ScalarType,
                public PlainTypeInstancePolicy<LglType> {
 public:
  std::string getName() const override {
    return "bool";
  }

 private:
  LglType() : ScalarType(kLogical) {}

  friend class PlainTypeInstancePolicy<LglType>;

  DISALLOW_COPY_AND_ASSIGN(LglType);
};


class IntType : public ScalarType,
                public PlainTypeInstancePolicy<IntType> {
 public:
  std::string getName() const override {
    return "int";
  }

 private:
  IntType() : ScalarType(kInteger) {}

  friend class PlainTypeInstancePolicy<IntType>;

  DISALLOW_COPY_AND_ASSIGN(IntType);
};


class DblType : public ScalarType,
                public PlainTypeInstancePolicy<DblType> {
 public:
  std::string getName() const override {
    return "double";
  }

 private:
  DblType() : ScalarType(kDouble) {}

  friend class PlainTypeInstancePolicy<DblType>;

  DISALLOW_COPY_AND_ASSIGN(DblType);
};


class StrType : public ScalarType,
                public PlainTypeInstancePolicy<StrType> {
 public:
  std::string getName() const override {
    return "string";
  }

 private:
  StrType() : ScalarType(kString) {}

  friend class PlainTypeInstancePolicy<StrType>;

  DISALLOW_COPY_AND_ASSIGN(StrType);
};


/**
 * @brief Unary parametric type.
 */
class ParametricType : public Type {
 public:
  bool equals(const Type &other) const override {
    if (getTypeID() != other.getTypeID()) {
      return false;
    }
    return parameter_.equals(
        static_cast<const ParametricType&>(other).parameter_);
  }

  std::size_t hash() const override {
    return CombineHashes(getTypeID(), parameter_.hash());
  }

  bool isParametric() const override {
    return true;
  }

  inline const Type& getParameter() const {
    return parameter_;
  }

 protected:
  ParametricType(const TypeID type_id, const Type &parameter)
      : Type(type_id),
        parameter_(parameter) {}

 private:
  const Type &parameter_;

  DISALLOW_COPY_AND_ASSIGN(ParametricType);
};


class VecType : public ParametricType,
                public ParametricTypeInstancePolicy<VecType> {
 public:
  std::string getName() const override {
    return "vector<" + getParameter().getName() + ">";
  }

 private:
  VecType(const Type &parameter)
      : ParametricType(kVector, parameter) {}

  friend class ParametricTypeInstancePolicy<VecType>;

  DISALLOW_COPY_AND_ASSIGN(VecType);
};

class MatType : public ParametricType,
                public ParametricTypeInstancePolicy<MatType> {
 public:
  std::string getName() const override {
    return "matrix<" + getParameter().getName() + ">";
  }

 private:
  MatType(const Type &parameter)
      : ParametricType(kMatrix, parameter) {}

  friend class ParametricTypeInstancePolicy<MatType>;

  DISALLOW_COPY_AND_ASSIGN(MatType);
};


class FunType : public Type ,
                public PlainTypeInstancePolicy<FunType> {
 public:
  std::string getName() const override {
    return "function";
  }

  bool equals(const Type &other) const override {
    return getTypeID() == other.getTypeID();
  }

  std::size_t hash() const override {
    return getTypeID();
  }

 private:
  FunType() : Type(kFunction) {}

  friend class PlainTypeInstancePolicy<FunType>;

  DISALLOW_COPY_AND_ASSIGN(FunType);
};


class AnyType : public Type,
                public PlainTypeInstancePolicy<AnyType> {
 public:
  static const AnyType& Instance() {
    static AnyType instance;
    return instance;
  }

  std::string getName() const override {
    return "any";
  }

  bool equals(const Type &other) const override {
    return getTypeID() == other.getTypeID();
  }

  std::size_t hash() const override {
    return getTypeID();
  }

 private:
  AnyType() : Type(kAny) {}

  friend class PlainTypeInstancePolicy<AnyType>;

  DISALLOW_COPY_AND_ASSIGN(AnyType);
};

class TypeFactory {
 public:
  static const Type& GetType(const TypeID type_id);

  static const Type& GetType(const TypeID type_id, const Type &parameter);

 private:
  TypeFactory() {}

  DISALLOW_COPY_AND_ASSIGN(TypeFactory);
};

}  // namespace rosa

#endif  // ROSA_TYPE_HPP_
