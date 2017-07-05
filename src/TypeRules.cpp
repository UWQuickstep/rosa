#include "TypeRules.hpp"

namespace rosa {
namespace rule {
namespace type {

TypeRule::~TypeRule() {
}

const Type& TypeRule::apply(const std::vector<const Type*> &args,
                            const ListPtr &values) const {
  return apply(args);
}

}  // namespace type
}  // namespace rule
}  // namespace rosa
