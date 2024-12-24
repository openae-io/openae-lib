#pragma once

#include <array>
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

template <typename Key, typename T>
class SingleEntryStorage {
public:
    T& insert(Key key, T result) {
        entry_.emplace(key, std::move(result));
        return entry_->second;
    }

    const T* find(Key key) const noexcept {
        if (entry_.has_value() && entry_->first == key) {
            return &entry_->second;
        }
        return nullptr;
    }

private:
    std::optional<std::pair<Key, T>> entry_;
};

template <typename Key, typename T, std::size_t N>
class RingBufferStorage {
public:
    std::size_t size() const noexcept {
        return size_;
    }

    T& insert(Key key, T result) {
        if (auto* existing_value = find_impl(*this, key)) {
            *existing_value = std::move(result);
            return *existing_value;
        }
        auto& entry = buffer_[write_index_];
        entry.first = key;
        entry.second = std::move(result);

        write_index_ = (write_index_ + 1) % N;
        if (size_ == N) {
            read_index_ = (read_index_ + 1) % N;  // overwrite last entry
        } else {
            size_++;
        }
        return entry.second;
    }

    const T* find(Key key) const noexcept {
        return find_impl(*this, key);
    }

private:
    template <typename Self>
    static auto* find_impl(Self&& self, Key key) noexcept {
        for (std::size_t i = 0; i < self.size(); ++i) {
            const auto index = (self.read_index_ + i) % N;
            auto& entry = self.buffer_[index];
            if (entry.first == key) {
                return &entry.second;
            }
        }
        // TODO: find cleaner solution
        if constexpr (std::is_const_v<std::remove_reference_t<Self>>) {
            return static_cast<const T*>(nullptr);
        } else {
            return static_cast<T*>(nullptr);
        }
    }

    std::array<std::pair<Key, T>, N> buffer_{};
    std::size_t size_{0};
    std::size_t write_index_{0};
    std::size_t read_index_{0};
};

template <template <typename> typename Storage, typename... Ts>
class BasicCache {
public:
    template <typename T>
    auto& insert(CacheKey key, T&& result) {
        using ValueType = std::remove_cvref_t<T>;
        assertIsSupportedType<ValueType>();
        return storage<ValueType>().insert(key, std::forward<T>(result));
    }

    template <typename T>
    const T* find(CacheKey key) const noexcept {
        using ValueType = std::remove_cvref_t<T>;
        assertIsSupportedType<ValueType>();
        return storage<ValueType>().find(key);
    }

private:
    template <typename T>
    static constexpr void assertIsSupportedType() {
        static_assert(
            (std::is_same_v<T, Ts> || ...),
            "Type not supported for caching. "
            "You may want to add the type to the type list of the Cache definition."
        );
    }

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

template <typename T>
using Storage = RingBufferStorage<CacheKey, T, 16>;

struct Cache : public BasicCache<Storage, bool, int, std::size_t, float, double> {};

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
