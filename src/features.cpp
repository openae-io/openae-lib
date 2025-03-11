#include "openae/features.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>  // round
#include <complex>
#include <concepts>
#include <cstddef>  // size_t
#include <iterator>
#include <limits>
#include <numeric>  // reduce
#include <ranges>
#include <vector>

#include "openae/common.hpp"

namespace {

template <std::floating_point T>
constexpr T quite_nan() noexcept {
    static_assert(std::numeric_limits<T>::has_quiet_NaN);
    return std::numeric_limits<T>::quiet_NaN();
}

/// Ìntegral-valued powers
/// @see https://en.wikipedia.org/wiki/Exponentiation_by_squaring
/// @see https://github.com/kthohr/gcem/blob/master/include%2Fgcem_incl%2Fpow_integral.hpp
template <size_t Exp, typename T>
constexpr T pow(T base) noexcept {
    if constexpr (Exp == 0) {
        return T{1};
    } else if constexpr (Exp % 2 == 0) {
        return pow<Exp / 2>(base) * pow<Exp / 2>(base);
    } else {
        return base * pow<Exp - 1>(base);
    }
}

template <typename T, std::ranges::input_range Range>
constexpr T sum(const Range& range) {
    return std::reduce(std::ranges::begin(range), std::ranges::end(range), T{0});
}

template <std::floating_point T, std::ranges::input_range Range>
constexpr T mean(const Range& range) {
    return sum<T>(range) / std::ranges::size(range);
}

template <std::floating_point T, std::ranges::input_range Range>
constexpr T geometric_mean(const Range& range) {
    T log_sum{0};
    for (const auto& value : range) {
        if (value == 0) {
            return T{0};
        }
        log_sum += std::log(value);
    }
    return std::exp(log_sum / std::ranges::size(range));
}

template <std::ranges::input_range Range>
constexpr auto max_element(const Range& range) {
    // with optimized version for views (lazy evaluation)
    if constexpr (std::ranges::view<Range>) {
        if (range.empty()) {
            return range.end();
        }
        auto it = range.begin();
        auto largest = range.begin();
        auto largest_value = *largest;  // cache largest value to avoid recomputation in lazy views
        while (++it != range.end()) {
            if (*it > largest_value) {
                largest = it;
                largest_value = *it;
            }
        }
        return largest;
    } else {
        return std::ranges::max_element(range);
    }
}

constexpr float bin_to_hz(float samplerate, std::integral auto bins, auto bin) noexcept {
    if (bins <= 1) {
        return quite_nan<float>();
    }
    // TODO: handle unexpected arguments
    // assert(static_cast<float>(bin) <= static_cast<float>(bins - 1));
    return 0.5F * samplerate * static_cast<float>(bin) / static_cast<float>(bins - 1);
}

constexpr size_t hz_to_bin(float samplerate, std::integral auto bins, float frequency) noexcept {
    if (samplerate == 0.0F || bins <= 1) {
        return 0;
    }
    // TODO: handle unexpected arguments
    assert(frequency >= 0.0F);
    assert(frequency <= 0.5F * samplerate);
    const auto bin = static_cast<float>(bins - 1) * frequency / (0.5F * samplerate);
    return static_cast<size_t>(std::round(bin));
}

namespace views {

template <std::ranges::input_range Range>
constexpr auto square(const Range& range) {
    return std::views::transform(range, [](auto v) { return v * v; });
}

template <std::ranges::input_range Range>
constexpr auto abs(const Range& range) {
    return std::views::transform(range, [](auto v) { return std::abs(v); });
}

template <std::ranges::input_range Range>
constexpr auto sqrt(const Range& range) {
    return std::views::transform(range, [](auto v) { return std::sqrt(v); });
}

}  // namespace views

}  // namespace

namespace openae::features {

using Timedata = decltype(Input::timedata);
using Spectrum = decltype(Input::spectrum);

inline static std::pmr::memory_resource* mem_resource_or_default(Env& env) noexcept {
    return env.mem_resource != nullptr ? env.mem_resource : std::pmr::get_default_resource();
}

/* -------------------------------------------- Basic ------------------------------------------- */

float peak_amplitude([[maybe_unused]] Env& env, Input input) {
    if (input.timedata.empty()) {
        return 0.0F;
    }
    const auto [min, max] = std::ranges::minmax(input.timedata);
    return std::max(std::abs(min), std::abs(max));
}

float energy([[maybe_unused]] Env& env, Input input) {
    return sum<float>(views::square(input.timedata)) / input.samplerate;
}

float rms([[maybe_unused]] Env& env, Input input) {
    return std::sqrt(mean<float>(views::square(input.timedata)));
}

float crest_factor([[maybe_unused]] Env& env, Input input) {
    return peak_amplitude(env, input) / rms(env, input);
}

float impulse_factor([[maybe_unused]] Env& env, Input input) {
    return peak_amplitude(env, input) / mean<float>(views::abs(input.timedata));
}

float clearance_factor([[maybe_unused]] Env& env, Input input) {
    return peak_amplitude(env, input) /
        pow<2>(mean<float>(views::sqrt(views::abs(input.timedata))));
}

float shape_factor([[maybe_unused]] Env& env, Input input) {
    return rms(env, input) / mean<float>(views::abs(input.timedata));
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
    return to_rate * static_cast<float>(zero_crossings(input.timedata));
}

/* ----------------------------------------- Statistics ----------------------------------------- */

template <size_t N>
static constexpr float central_moment(Timedata y, float y_mean) {
    return mean<float>(std::views::transform(y, [y_mean](auto v) { return pow<N>(v - y_mean); }));
}

template <size_t N>
static constexpr float standardized_moment(Timedata y) {
    if (y.size() < N) {
        return quite_nan<float>();
    }
    const auto y_mean = mean<float>(y);
    return central_moment<N>(y, y_mean) / pow<N>(std::sqrt(central_moment<2>(y, y_mean)));
}

float skewness([[maybe_unused]] Env& env, Input input) {
    return standardized_moment<3>(input.timedata);
}

float kurtosis([[maybe_unused]] Env& env, Input input) {
    return standardized_moment<4>(input.timedata);
}

/* ------------------------------------------ Spectral ------------------------------------------ */

[[maybe_unused]] static std::pmr::vector<float> power_spectrum_allocate(Env& env, Input input) {
    const auto bins = input.spectrum.size();
    std::pmr::vector<float> power_spectrum(bins, mem_resource_or_default(env));
    std::transform(
        input.spectrum.begin(),
        input.spectrum.end(),
        power_spectrum.begin(),
        [](std::complex<float> c) { return std::norm(c); }
    );
    return power_spectrum;
}

static constexpr auto power_spectrum_view(Spectrum spectrum) {
    return std::views::transform(spectrum, [](auto c) { return std::norm(c); });
}

float partial_power([[maybe_unused]] Env& env, Input input, float fmin, float fmax) {
    fmin = std::clamp(fmin, 0.0F, 0.5F * input.samplerate);
    fmax = std::clamp(fmax, fmin, 0.5F * input.samplerate);
    const auto power_spectrum = power_spectrum_view(input.spectrum);
    const auto power_spectrum_range = std::ranges::subrange(
        // NOLINTBEGIN(*narrowing-conversions)
        power_spectrum.begin() + hz_to_bin(input.samplerate, power_spectrum.size(), fmin),
        power_spectrum.begin() + hz_to_bin(input.samplerate, power_spectrum.size(), fmax)
        // NOLINTEND(*narrowing-conversions)
    );
    return sum<float>(power_spectrum_range) / sum<float>(power_spectrum);
}

float spectral_peak_frequency([[maybe_unused]] Env& env, Input input) {
    const auto power_spectrum = power_spectrum_view(input.spectrum);
    const auto it = max_element(power_spectrum);
    if (it == power_spectrum.end()) {
        return quite_nan<float>();
    }
    const auto bin = std::distance(power_spectrum.begin(), it);
    return bin_to_hz(input.samplerate, power_spectrum.size(), bin);
}

float spectral_centroid([[maybe_unused]] Env& env, Input input) {
    // TODO: workaround to prevent bin = 0 / 0, which returns NOT NaN with MSVC
    if (input.spectrum.empty()) {
        return quite_nan<float>();
    }
    const auto power_spectrum = power_spectrum_view(input.spectrum);
    const auto bins = power_spectrum.size();
    float power_sum = 0.0F;
    float power_sum_weighted = 0.0F;
    for (size_t bin = 0; bin < bins; ++bin) {
        const auto power = power_spectrum[static_cast<std::ptrdiff_t>(bin)];
        power_sum += power;
        power_sum_weighted += power * static_cast<float>(bin);
    }
    return bin_to_hz(input.samplerate, bins, power_sum_weighted / power_sum);
}

template <size_t N>
static float spectral_central_moment([[maybe_unused]] Env& env, Input input, float f_centroid) {
    const auto power_spectrum = power_spectrum_view(input.spectrum);
    const auto bins = power_spectrum.size();
    const auto factor_bin_to_hz = bin_to_hz(input.samplerate, bins, 1);
    float power_sum = 0.0F;
    float power_sum_weighted = 0.0F;
    for (size_t bin = 0; bin < bins; ++bin) {
        const auto power = power_spectrum[static_cast<std::ptrdiff_t>(bin)];
        const auto f = factor_bin_to_hz * static_cast<float>(bin);
        power_sum += power;
        power_sum_weighted += power * pow<N>(f - f_centroid);
    }
    return power_sum_weighted / power_sum;
}

template <size_t N>
static float spectral_standardized_moment([[maybe_unused]] Env& env, Input input) {
    const auto f_centroid = spectral_centroid(env, input);
    return spectral_central_moment<N>(env, input, f_centroid) /
        pow<N>(std::sqrt(spectral_central_moment<2>(env, input, f_centroid)));
}

float spectral_variance(Env& env, Input input) {
    return spectral_central_moment<2>(env, input, spectral_centroid(env, input));
}

float spectral_skewness(Env& env, Input input) {
    return spectral_standardized_moment<3>(env, input);
}

float spectral_kurtosis(Env& env, Input input) {
    return spectral_standardized_moment<4>(env, input);
}

float spectral_rolloff(Env& env, Input input, float rolloff) {
    if (input.spectrum.empty()) {
        return 0.0F;
    }
    const auto power_spectrum = power_spectrum_view(input.spectrum);
    std::pmr::vector<float> acc(power_spectrum.size(), mem_resource_or_default(env));
    std::partial_sum(power_spectrum.begin(), power_spectrum.end(), acc.begin());

    const auto total = acc.back();
    const auto threshold = total * std::clamp(rolloff, 0.0F, 1.0F);

    const auto it = std::upper_bound(acc.begin(), acc.end(), threshold);
    const auto bin = std::distance(acc.begin(), it);
    return bin_to_hz(input.samplerate, input.spectrum.size(), bin);
}

float spectral_entropy([[maybe_unused]] Env& env, Input input) {
    const auto power_spectrum = power_spectrum_view(input.spectrum);
    float power_sum = 0.0F;
    float power_sum_weighted = 0.0F;
    for (const auto power : power_spectrum) {
        power_sum += power;
        if (power > 0.0F) {
            power_sum_weighted += power * std::log2(power);
        }
    }
    if (power_sum == 0.0F || power_spectrum.size() <= 1) {
        return 0.0F;
    }
    const auto entropy = std::log2(power_sum) - (power_sum_weighted / power_sum);
    return entropy / std::log2(static_cast<float>(power_spectrum.size()));
}

float spectral_flatness([[maybe_unused]] Env& env, Input input) {
    const auto power_spectrum = power_spectrum_view(input.spectrum);
    return geometric_mean<float>(power_spectrum) / mean<float>(power_spectrum);
}

}  // namespace openae::features
