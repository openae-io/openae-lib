#include "openae/features.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>  // accumulate
#include <ranges>

#include "openae/common.hpp"

#include "cache.hpp"
#include "hash.hpp"
#include "features_meta.hpp"

namespace openae::features {

using Timedata = std::span<const float>;
using Spectrum = std::span<const std::complex<float>>;

/// Ìntegral-valued powers
/// @see https://en.wikipedia.org/wiki/Exponentiation_by_squaring
/// @see https://github.com/kthohr/gcem/blob/master/include%2Fgcem_incl%2Fpow_integral.hpp
template <size_t Exp, typename T>
static constexpr T pow(T base) noexcept {
    if constexpr (Exp == 0) {
        return T{1};
    } else if constexpr (Exp % 2 == 0) {
        return pow<Exp / 2>(base) * pow<Exp / 2>(base);
    } else {
        return base * pow<Exp - 1>(base);
    }
}

template <typename T, typename... Args>
static constexpr T accumulate(std::span<const T> y, T init, Args... args) {
    return std::accumulate(y.begin(), y.end(), init, args...);
}

template <typename T>
static constexpr T avg(std::span<const T> y) {
    return std::accumulate(y.begin(), y.end(), T{0}) / static_cast<T>(y.size());
}

template <typename T, typename UnaryOp>
static constexpr T transform_avg(std::span<const T> y, UnaryOp unary_op) {
    if (y.empty()) {
        return T{0};
    };
    const auto binary_op = [unary_op](auto sum, auto v) { return sum + unary_op(v); };
    return std::accumulate(y.begin(), y.end(), T{0}, binary_op) / static_cast<T>(y.size());
}

static constexpr float bin_to_hz(Input input, size_t index) noexcept {
    const auto nyquist = 0.5f * input.samplerate;
    const auto nfft = input.spectrum.size();
    return nyquist * static_cast<float>(index) / static_cast<float>(nfft - 1);
}

/* -------------------------------------------- Basic ------------------------------------------- */

float peak_amplitude([[maybe_unused]] Env& env, Input input) {
    const auto [min, max] = std::minmax_element(input.timedata.begin(), input.timedata.end());
    return std::max(
        std::abs(min != input.timedata.end() ? *min : 0.0f),
        std::abs(max != input.timedata.end() ? *max : 0.0f)
    );
}

float rms([[maybe_unused]] Env& env, Input input) {
    return std::sqrt(transform_avg(input.timedata, [](auto v) { return v * v; }));
}

float crest_factor([[maybe_unused]] Env& env, Input input) {
    return peak_amplitude(env, input) / rms(env, input);
}

static float mean_absolute(Timedata y) {
    return transform_avg(y, [](auto v) { return std::abs(v); });
}

float impulse_factor([[maybe_unused]] Env& env, Input input) {
    return peak_amplitude(env, input) / mean_absolute(input.timedata);
}

float k_factor([[maybe_unused]] Env& env, Input input) {
    return peak_amplitude(env, input) * rms(env, input);
}

float margin_factor([[maybe_unused]] Env& env, Input input) {
    const auto mean_sqrt_abs = transform_avg(input.timedata, [](auto v) {
        return std::sqrt(std::abs(v));
    });
    return peak_amplitude(env, input) / (mean_sqrt_abs * mean_sqrt_abs);
}

float shape_factor([[maybe_unused]] Env& env, Input input) {
    return rms(env, input) / mean_absolute(input.timedata);
}

static size_t zero_crossings(Timedata y) {
    const auto is_positive = [](auto v) { return v >= 0; };
    size_t crossings = 0;
    for (size_t i = 1; i < y.size(); ++i) {
        if (is_positive(y[i - 1]) != is_positive(y[i])) {
            ++crossings;
        }
    }
    return crossings;
}

float zero_crossing_rate([[maybe_unused]] Env& env, Input input) {
    const auto to_rate = input.samplerate / static_cast<float>(input.timedata.size());
    return to_rate * zero_crossings(input.timedata);
}

/* ----------------------------------------- Statistics ----------------------------------------- */

template <size_t N>
static float central_moment(Timedata y, float mean) {
    return transform_avg(y, [mean](auto v) { return pow<N>(v - mean); });
}

template <size_t N>
static float standardized_moment(Timedata y) {
    if (y.size() < N) {
        return 0.0f;
    }
    const auto mean = avg(y);
    const auto variance = central_moment<2>(y, mean);
    return central_moment<N>(y, mean) / pow<N>(std::sqrt(variance));
}

float skewness([[maybe_unused]] Env& env, Input input) {
    return standardized_moment<3>(input.timedata);
}

float kurtosis([[maybe_unused]] Env& env, Input input) {
    return standardized_moment<4>(input.timedata);
}

/* ------------------------------------------ Spectral ------------------------------------------ */

static std::pmr::vector<float> power_spectrum_allocate(Env& env, Input input) {
    const auto bins = input.spectrum.size();
    std::pmr::vector<float> power_spectrum(bins, env.mem_resource);
    std::transform(
        input.spectrum.begin(),
        input.spectrum.end(),
        power_spectrum.begin(),
        [](std::complex<float> c) { return std::norm(c); }
    );
    return power_spectrum;
}

static auto power_spectrum_view(Spectrum spectrum) {
    return std::ranges::views::transform(spectrum, [](auto c) { return std::norm(c); });
}

float spectral_centroid(Env& env, Input input) {
    const auto power_spectrum = power_spectrum_allocate(env, input);
    const auto bins = power_spectrum.size();
    float power_sum = 0;
    float power_sum_weighted = 0;
    for (size_t bin = 0; bin < bins; ++bin) {
        power_sum += power_spectrum[bin];
        power_sum_weighted += bin * power_spectrum[bin];
    }
    return 0.5f * input.samplerate / (bins - 1) * (power_sum / power_sum_weighted);
}

float spectral_centroid_lazy([[maybe_unused]] Env& env, Input input) {
    const auto power_spectrum = power_spectrum_view(input.spectrum);
    const auto bins = power_spectrum.size();
    float power_sum = 0;
    float power_sum_weighted = 0;
    for (size_t bin = 0; bin < bins; ++bin) {
        power_sum += power_spectrum[bin];
        power_sum_weighted += bin * power_spectrum[bin];
    }
    return 0.5f * input.samplerate / (bins - 1) * (power_sum / power_sum_weighted);
}

float spectral_centroid_inplace([[maybe_unused]] Env& env, Input input) {
    const auto bins = input.spectrum.size();
    float power_sum = 0;
    float power_sum_weighted = 0;
    for (size_t bin = 0; bin < bins; ++bin) {
        const auto power = std::norm(input.spectrum[bin]);
        power_sum += power;
        power_sum_weighted += bin * power;
    }
    return 0.5f * input.samplerate / (bins - 1) * (power_sum / power_sum_weighted);
}

float spectral_rolloff([[maybe_unused]] Env& env, Input input, float rolloff) {
    if (input.spectrum.empty()) {
        return 0.0f;
    }
    const auto power_spectrum = power_spectrum_view(input.spectrum);
    std::pmr::vector<float> acc(power_spectrum.size(), env.mem_resource);
    std::partial_sum(power_spectrum.begin(), power_spectrum.end(), acc.begin());

    const auto total = acc.back();
    const auto threshold = total * std::clamp(rolloff, 0.0f, 1.0f);

    const auto it = std::upper_bound(acc.begin(), acc.end(), threshold);
    const auto index = std::distance(acc.begin(), it);
    return bin_to_hz(input, index);
}

}  // namespace openae::features
