#pragma once

#include <cstddef>
#include <span>

#define XXH_INLINE_ALL
#include <xxhash.h>

#include "openae/common.hpp"
#include "openae/features.hpp"

namespace openae {

// https://www.boost.org/doc/libs/1_87_0/libs/container_hash/doc/html/hash.html#notes_hash_combine
template <typename T, typename... Rest>
inline void hash_combine(size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hash_value{};
    seed ^= hash_value(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    if constexpr (sizeof...(Rest) > 0) {
        hash_combine(seed, rest...);
    }
}

}  // namespace openae

namespace std {

template <>
struct std::hash<openae::Env> {
    size_t operator()([[maybe_unused]] const openae::Env& env) const noexcept {
        return 0;
    }
};

template <typename T>
struct std::hash<std::span<T>> {
    size_t operator()(std::span<T> arr) const noexcept {
#ifdef XXH128
        return XXH128(arr.data(), arr.size() * sizeof(T), arr.size() /* seed */).low64;
#else
        return XXH64(arr.data(), arr.size() * sizeof(T), arr.size() /* seed */);
#endif
    }
};

template <>
struct std::hash<openae::features::Input> {
    size_t operator()(const openae::features::Input& input) const noexcept {
        size_t seed{};
        openae::hash_combine(seed, input.samplerate);
        openae::hash_combine(seed, input.timedata);
        // openae::hash_combine(seed, input.spectrum);
        return seed;
    }
};

}  // namespace std
