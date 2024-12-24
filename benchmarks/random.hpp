#pragma once

#include <complex>
#include <random>
#include <type_traits>
#include <vector>

template <typename T, typename = void>
struct UniformDistribution;

template <typename T>
struct UniformDistribution<T, std::enable_if_t<std::is_integral_v<T>>> {
    constexpr UniformDistribution(T min, T max)
        : base(min, max) {}

    template <typename Engine>
    T operator()(Engine& engine) {
        return base(engine);
    }

    std::uniform_int_distribution<T> base;
};

template <typename T>
struct UniformDistribution<T, std::enable_if_t<std::is_floating_point_v<T>>> {
    constexpr UniformDistribution(T min, T max)
        : base(min, max) {}

    template <typename Engine>
    T operator()(Engine& engine) {
        return base(engine);
    }

    std::uniform_real_distribution<T> base;
};

template <typename T>
struct UniformDistribution<std::complex<T>> {
    constexpr UniformDistribution(std::complex<T> min, std::complex<T> max)
        : base_real(min.real(), max.real()),
          base_imag(min.imag(), max.imag()) {}

    template <typename Engine>
    std::complex<T> operator()(Engine& engine) {
        return {base_real(engine), base_imag(engine)};
    }

    UniformDistribution<T> base_real;
    UniformDistribution<T> base_imag;
};

template <typename T>
static std::vector<T> make_random_vector(size_t size, T min, T max) {
    std::random_device device;
    std::default_random_engine engine{device()};
    UniformDistribution<T> dist(min, max);

    std::vector<T> result(size);
    std::generate(result.begin(), result.end(), [&]() { return dist(engine); });
    return result;
}
