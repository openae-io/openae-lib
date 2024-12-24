#pragma once

#include <cstddef>
#include <functional>  // invoke
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>  // move, forward

#include "hash.hpp"

namespace openae {

struct CacheKey {
    std::size_t hash_func;
    std::size_t hash_args;
    auto operator<=>(const CacheKey&) const = default;
};

template <typename T>
class SingleCacheEntryStorage {
public:
    T& insert(CacheKey key, const T& result) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        entry_.emplace(key, result);
        return entry_->value;
    }

    T& insert(CacheKey key, T&& result) noexcept(std::is_nothrow_move_assignable_v<T>) {
        entry_.emplace(key, std::move(result));
        return entry_->value;
    }

    const T* find(CacheKey key) const noexcept {
        if (entry_.has_value() && entry_->key == key) {
            return &entry_->value;
        }
        return nullptr;
    }

private:
    struct Entry {
        CacheKey key;
        T value;
    };

    std::optional<Entry> entry_;
};

template <template <typename> typename Storage, typename... Ts>
class BasicCache {
public:
    template <typename T>
    auto& insert(CacheKey key, T&& result) {
        using ValueType = std::remove_cvref_t<T>;
        return storage<ValueType>().insert(key, std::forward<T>(result));
    }

    template <typename T>
    const T* find(CacheKey key) const noexcept {
        using ValueType = std::remove_cvref_t<T>;
        return storage<ValueType>().find(key);
    }

private:
    template <typename T>
    constexpr Storage<T>& storage() noexcept {
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>);
        return std::get<Storage<T>>(caches_);
    }

    template <typename T>
    constexpr const Storage<T>& storage() const noexcept {
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>);
        return std::get<Storage<T>>(caches_);
    }

    std::tuple<Storage<Ts>...> caches_;
};

struct Cache : public BasicCache<SingleCacheEntryStorage, bool, int, std::size_t, float, double> {};

// TODO: avoid expensive copies with proxy object for cached values.
template <typename Cache, typename Func, typename... Args>
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

    auto* value_ptr = cache->template find<ResultType>(key);
    if (value_ptr != nullptr) {
        return *value_ptr;
    }
    return cache->insert(key, invoke());
}

}  // namespace openae
