#include "Type.hpp"

#include "Macros.hpp"

namespace rosa {

bool Type::isScalar() const {
  return false;
}

bool Type::isParametric() const {
  return false;
}


const Type& Type::JoinType(const Type &lhs, const Type &rhs) {
  const Type *lo, *hi;
  if (lhs.getTypeID() <= rhs.getTypeID()) {
    lo = &lhs;
    hi = &rhs;
  } else {
    lo = &rhs;
    hi = &lhs;
  }

  switch (hi->getTypeID()) {
    case kNull:  // Fall through
    case kLogical:
    case kInteger:
    case kDouble:
    case kString:
    case kAny:
      return *hi;
    case kVector:  // Fall through
    case kMatrix: {
      if (lo->isScalar()) {
         return TypeFactory::GetType(
             hi->getTypeID(),
             JoinType(*lo, static_cast<const ParametricType*>(hi)->getParameter()));
      } else {
         return TypeFactory::GetType(
             hi->getTypeID(),
             JoinType(static_cast<const ParametricType*>(lo)->getParameter(),
                      static_cast<const ParametricType*>(hi)->getParameter()));

      }
    }
    case kFunction: {
      if (lo->getTypeID() == kNull || lo->getTypeID() == kFunction) {
        return *hi;
      } else {
        return AnyType::Instance();
      }
    }
    default:
      break;
  }

  FATAL_ERROR("Reaches unexpected location in JoinType()");
}

const Type& TypeFactory::GetType(const TypeID type_id) {
  switch (type_id) {
    case kNull:     return NilType::Instance();
    case kLogical:  return LglType::Instance();
    case kInteger:  return IntType::Instance();
    case kDouble:   return DblType::Instance();
    case kString:   return StrType::Instance();
    case kFunction: return FunType::Instance();
    case kAny:      return AnyType::Instance();
    default:
      break;
  }
  FATAL_ERROR("Unexpected type id in TypeFactory::Get(const TypeID)");
}

const Type& TypeFactory::GetType(const TypeID type_id,
                                 const Type &parameter) {
  switch (type_id) {
    case kVector: return VecType::Instance(parameter);
    case kMatrix: return MatType::Instance(parameter);
    default:
      break;
  }
  FATAL_ERROR("Unexpected type id in TypeFactory::Get(const TypeID, const Type&)");
}



}  // namespace rosa
