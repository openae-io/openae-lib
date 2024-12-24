#pragma once

#include <cstddef>
#include <functional>  // invoke
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>  // move, forward

#include "hash.hpp"

namespace openae {

template <typename T>
struct CacheEntry {
    size_t hash;
    T value;
};

template <typename T>
class SingleCacheEntryStorage {
public:
    T& insert(size_t hash, const T& result) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        entry_.emplace(hash, result);
        return entry_->value;
    }

    T& insert(size_t hash, T&& result) noexcept(std::is_nothrow_move_assignable_v<T>) {
        entry_.emplace(hash, std::move(result));
        return entry_->value;
    }

    const T* find(size_t hash) const noexcept {
        if (entry_.has_value() && entry_->hash == hash) {
            return &entry_->value;
        }
        return nullptr;
    }

private:
    std::optional<CacheEntry<T>> entry_;
};

template <template <typename> typename Storage, typename... Ts>
class BasicCache {
public:
    template <typename T>
    auto& insert(size_t hash, T&& result) {
        using ValueType = std::remove_cvref_t<T>;
        return storage<ValueType>().insert(hash, std::forward<T>(result));
    }

    template <typename T>
    const T* find(size_t hash) const noexcept {
        using ValueType = std::remove_cvref_t<T>;
        return storage<ValueType>().find(hash);
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

struct Cache : public BasicCache<SingleCacheEntryStorage, bool, int, size_t, float, double> {};

// TODO: avoid expensive copies with proxy object for cached values.
template <typename Cache, typename Func, typename... Args>
auto cached(Cache* cache, Func func, Args&&... args) {
    static_assert(std::is_invocable_v<Func, Args...>);
    using ResultType = std::remove_cvref_t<std::invoke_result_t<Func, Args...>>;

    const auto invoke = [&] { return std::invoke(func, std::forward<Args>(args)...); };
    if (cache == nullptr) {
        return invoke();
    }

    size_t hash{};
    hash_combine(hash, args...);
    auto* value_ptr = cache->template find<ResultType>(hash);
    if (value_ptr != nullptr) {
        return *value_ptr;
    }
    return cache->insert(hash, invoke());
}

}  // namespace openae
