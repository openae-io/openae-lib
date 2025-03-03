#pragma once

#include <complex>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>

#include "openae/config.hpp"
#include "openae/common.hpp"

namespace openae::features {

struct Input {
    float samplerate;
    std::span<const float> timedata;
    std::span<const std::complex<float>> spectrum;
    std::optional<std::size_t> fingerprint;
};

OPENAE_EXPORT float peak_amplitude(Env& env, Input input);
OPENAE_EXPORT float energy(Env& env, Input input);
OPENAE_EXPORT float rms(Env& env, Input input);
OPENAE_EXPORT float crest_factor(Env& env, Input input);
OPENAE_EXPORT float impulse_factor(Env& env, Input input);
OPENAE_EXPORT float clearance_factor(Env& env, Input input);
OPENAE_EXPORT float shape_factor(Env& env, Input input);
OPENAE_EXPORT float skewness(Env& env, Input input);
OPENAE_EXPORT float kurtosis(Env& env, Input input);
OPENAE_EXPORT float zero_crossing_rate(Env& env, Input input);
OPENAE_EXPORT float partial_power(Env& env, Input input, float fmin, float fmax);
OPENAE_EXPORT float spectral_peak_frequency(Env& env, Input input);
OPENAE_EXPORT float spectral_centroid(Env& env, Input input);
OPENAE_EXPORT float spectral_variance(Env& env, Input input);
OPENAE_EXPORT float spectral_skewness(Env& env, Input input);
OPENAE_EXPORT float spectral_kurtosis(Env& env, Input input);
OPENAE_EXPORT float spectral_rolloff(Env& env, Input input, float rolloff);
OPENAE_EXPORT float spectral_flatness(Env& env, Input input);

}  // namespace openae::features
