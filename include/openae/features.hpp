#pragma once

#include <complex>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>

#include "openae/common.hpp"

namespace openae::features {

struct Input {
    float samplerate;
    std::span<const float> timedata;
    std::span<const std::complex<float>> spectrum;
    std::optional<std::size_t> fingerprint;
};

/* ------------------------------------------ Functions ----------------------------------------- */

float peak_amplitude(Env& env, Input input);
float energy(Env& env, Input input);
float rms(Env& env, Input input);
float crest_factor(Env& env, Input input);
float impulse_factor(Env& env, Input input);
float clearance_factor(Env& env, Input input);
float shape_factor(Env& env, Input input);

float skewness(Env& env, Input input);
float kurtosis(Env& env, Input input);

float zero_crossing_rate(Env& env, Input input);

float partial_power(Env& env, Input input, float fmin, float fmax);
float spectral_peak_frequency(Env& env, Input input);
float spectral_centroid(Env& env, Input input);
float spectral_variance(Env& env, Input input);
float spectral_skewness(Env& env, Input input);
float spectral_kurtosis(Env& env, Input input);
float spectral_rolloff(Env& env, Input input, float rolloff);

}  // namespace openae::features
