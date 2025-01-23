#include "openae/features.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <numeric>  // reduce
#include <ranges>
#include <type_traits>

#include "openae/common.hpp"

#include "cache.hpp"
#include "features_meta.hpp"
#include "hash.hpp"

namespace {

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

template <std::ranges::input_range Range>
constexpr auto sum(const Range& range) {
    return std::reduce(std::ranges::begin(range), std::ranges::end(range));
}

template <std::ranges::input_range Range>
constexpr auto mean(const Range& range) {
    return sum(range) / std::ranges::size(range);
}

inline constexpr float bin_to_hz(float samplerate, size_t bins, auto bin) noexcept {
    // TODO: handle unexpected arguments
    assert(static_cast<size_t>(bin) < bins);
    if (bins <= 1) {
        return 0.0f;
    }
    return 0.5f * samplerate * static_cast<float>(bin) / static_cast<float>(bins - 1);
}

inline constexpr size_t hz_to_bin(float samplerate, size_t bins, float frequency) noexcept {
    // TODO: handle unexpected arguments
    if (samplerate == 0.0f) {
        return 0;
    }
    assert(frequency >= 0.0f);
    assert(frequency <= 0.5f * samplerate);
    const auto bin = static_cast<float>(bins - 1) * frequency / (0.5f * samplerate);
    return static_cast<size_t>(bin + 0.5f);
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

/* -------------------------------------------- Basic ------------------------------------------- */

float peak_amplitude([[maybe_unused]] Env& env, Input input) {
    if (input.timedata.empty()) {
        return 0.0f;
    }
    const auto [min, max] = std::ranges::minmax(input.timedata);
    return std::max(std::abs(min), std::abs(max));
}

float energy([[maybe_unused]] Env& env, Input input) {
    return sum(views::square(input.timedata)) / input.samplerate;
}

float rms([[maybe_unused]] Env& env, Input input) {
    return std::sqrt(mean(views::square(input.timedata)));
}

float crest_factor([[maybe_unused]] Env& env, Input input) {
    return peak_amplitude(env, input) / rms(env, input);
}

float impulse_factor([[maybe_unused]] Env& env, Input input) {
    return peak_amplitude(env, input) / mean(views::abs(input.timedata));
}

float clearance_factor([[maybe_unused]] Env& env, Input input) {
    return peak_amplitude(env, input) / pow<2>(mean(views::sqrt(views::abs(input.timedata))));
}

float shape_factor([[maybe_unused]] Env& env, Input input) {
    return rms(env, input) / mean(views::abs(input.timedata));
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
static float central_moment(Timedata y, float y_mean) {
    return mean(std::views::transform(y, [y_mean](auto v) { return pow<N>(v - y_mean); }));
}

template <size_t N>
static float standardized_moment(Timedata y) {
    if (y.size() < N) {
        return 0.0f;
    }
    const auto y_mean = mean(y);
    const auto y_variance = central_moment<2>(y, y_mean);
    return central_moment<N>(y, y_mean) / pow<N>(std::sqrt(y_variance));
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
    return std::views::transform(spectrum, [](auto c) { return std::norm(c); });
}

float partial_power([[maybe_unused]] Env& env, Input input, float fmin, float fmax) {
    fmin = std::clamp(fmin, 0.0f, 0.5f * input.samplerate);
    fmax = std::clamp(fmax, fmin, 0.5f * input.samplerate);
    const auto power_spectrum = power_spectrum_view(input.spectrum);
    const auto power_spectrum_range = std::ranges::subrange(
        power_spectrum.begin() + hz_to_bin(input.samplerate, power_spectrum.size(), fmin),
        power_spectrum.begin() + hz_to_bin(input.samplerate, power_spectrum.size(), fmax)
    );
    return sum(power_spectrum_range) / sum(power_spectrum);
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
    return bin_to_hz(input.samplerate, bins, power_sum_weighted / power_sum);
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
    return bin_to_hz(input.samplerate, bins, power_sum_weighted / power_sum);
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
    return bin_to_hz(input.samplerate, bins, power_sum_weighted / power_sum);
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
    const auto bin = std::distance(acc.begin(), it);
    return bin_to_hz(input.samplerate, input.spectrum.size(), bin);
}

}  // namespace openae::features
