#pragma once

#include <array>
#include <cstddef>
#include <functional>  // invoke, hash
#include <tuple>
#include <type_traits>
#include <utility>  // as_const, forward, move, pair

#include "hash.hpp"

namespace openae {

/// Fixed-capacity FIFO storage with linear-scan lookup.
template <typename Key, typename T, std::size_t N = 16>
class RingBufferStorage {
public:
    std::size_t size() const noexcept {
        return size_;
    }

    T& insert(Key key, T value) {
        if (auto* existing = find(key)) {
            *existing = std::move(value);
            return *existing;
        }
        auto& entry = buffer_[write_];
        entry.first = key;
        entry.second = std::move(value);
        write_ = (write_ + 1) % N;
        if (size_ == N) {
            read_ = (read_ + 1) % N;  // overwrite oldest entry
        } else {
            ++size_;
        }
        return entry.second;
    }

    const T* find(Key key) const noexcept {
        for (std::size_t i = 0; i < size_; ++i) {
            const auto idx = (read_ + i) % N;
            if (buffer_[idx].first == key) {
                return &buffer_[idx].second;
            }
        }
        return nullptr;
    }

    T* find(Key key) noexcept {
        return const_cast<T*>(std::as_const(*this).find(key));
    }

private:
    std::array<std::pair<Key, T>, N> buffer_{};
    std::size_t size_{0};
    std::size_t write_{0};
    std::size_t read_{0};
};

/// Cache key: hashed function identity + hashed arguments.
struct CacheKey {
    std::size_t hash_func;
    std::size_t hash_args;
    auto operator<=>(const CacheKey&) const = default;
};

struct Cache {
    template <typename T>
    using Storage = RingBufferStorage<CacheKey, T>;

    std::tuple<Storage<int>, Storage<float>> storages;

    template <typename T>
    const T* find(CacheKey key) const noexcept {
        return std::get<Storage<T>>(storages).find(key);
    }

    template <typename T>
    T& insert(CacheKey key, T value) {
        return std::get<Storage<T>>(storages).insert(key, std::move(value));
    }
};

template <typename Func, typename... Args>
auto cached(Cache* cache, Func func, Args&&... args) {
    static_assert(std::is_invocable_v<Func, Args...>);
    using ResultType = std::remove_cvref_t<std::invoke_result_t<Func, Args...>>;

    const auto invoke = [&] { return std::invoke(func, std::forward<Args>(args)...); };
    if (cache == nullptr) {
        return invoke();
    }

    CacheKey key{
        .hash_func = std::hash<Func>{}(func),
        .hash_args = {},
    };
    hash_combine(key.hash_args, args...);

    if (const auto* value_ptr = cache->template find<ResultType>(key)) {
        return *value_ptr;
    }
    return cache->insert(key, invoke());
}

}  // namespace openae
