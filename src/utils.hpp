#pragma once

#include <utility>

namespace openae {

template <typename Tuple, typename F, std::size_t... I>
constexpr void for_each_impl(Tuple&& tuple, F&& f, std::index_sequence<I...>) {
    (std::forward<F>(f)(std::get<I>(tuple)), ...);
}

template <typename Tuple, typename F>
constexpr void for_each(Tuple&& tuple, F&& f) {
    for_each_impl(
        tuple,
        std::forward<F>(f),
        std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{}
    );
}

/**
 * Wrapper with deleted default constructor to require explicit initialization.
 * This is useful when you want to prevent the default initialization of an array:
 * @code
 * std::array<int, 3> arr{1, 2};  // compiles
 * std::array<RequireInit<int>, 3> arr{1, 2};  // compile error
 * @endcode
 */
template <typename T>
struct RequireInit {
    RequireInit() = delete;

    constexpr RequireInit(const T& v)
        : value(v) {}

    constexpr RequireInit(T&& v)
        : value(std::move(v)) {}

    T value;
};

}  // namespace openae
