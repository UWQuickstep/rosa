#ifndef ROSA_HASH_UTIL_HPP_
#define ROSA_HASH_UTIL_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

namespace rosa {

namespace hash_combine_detail {

template <typename HashT, typename Enable = void>
struct HashCombiner;

// Specialization for 64-bit size_t.
template <typename HashT>
struct HashCombiner<HashT,
                    typename std::enable_if<std::is_same<HashT, std::size_t>::value
                                            && (sizeof(HashT) == 8)>::type> {
  static inline std::size_t CombineHashes(std::size_t first_hash,
                                          std::size_t second_hash) {
    // Based on Hash128to64() from cityhash.
    static constexpr std::size_t kMul = UINT64_C(0x9ddfea08eb382d69);
    HashT a = (first_hash ^ second_hash) * kMul;
    a ^= (a >> 47);
    HashT b = (second_hash ^ a) * kMul;
    b ^= (b >> 47);
    b *= kMul;
    return b;
  }
};

// Specialization for 32-bit size_t.
template <typename HashT>
struct HashCombiner<HashT,
                    typename std::enable_if<std::is_same<HashT, std::size_t>::value
                                            && sizeof(HashT) == 4>::type> {
  static inline std::size_t CombineHashes(std::size_t first_hash,
                                          std::size_t second_hash) {
    // Based on hash_combine from Boost.
    first_hash ^= second_hash + 0x9e3779b9u + (first_hash << 6) + (first_hash >> 2);
    return first_hash;
  }
};

}  // namespace hash_combine_detail

inline std::size_t CombineHashes(std::size_t first_hash,
                                 std::size_t second_hash) {
  return hash_combine_detail::HashCombiner<std::size_t>::CombineHashes(first_hash, second_hash);
}

}  // namespace rosa

#endif  // ROSA_HASH_UTIL_HPP_
