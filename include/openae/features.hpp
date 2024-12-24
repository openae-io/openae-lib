#pragma once

#include <complex>
#include <memory>
#include <optional>
#include <span>

#include "openae/common.hpp"

namespace openae::features {

struct Input {
    float samplerate;
    std::span<const float> timedata;
    std::span<const std::complex<float>> spectrum;
};

/* ------------------------------------------ Functions ----------------------------------------- */

float peak_amplitude(Env& env, Input input);
float rms(Env& env, Input input);
float crest_factor(Env& env, Input input);
float impulse_factor(Env& env, Input input);
float k_factor(Env& env, Input input);
float margin_factor(Env& env, Input input);
float shape_factor(Env& env, Input input);

float skewness(Env& env, Input input);
float kurtosis(Env& env, Input input);

float zero_crossing_rate(Env& env, Input input);

float spectral_centroid(Env& env, Input input);
float spectral_centroid_lazy(Env& env, Input input);
float spectral_centroid_inplace(Env& env, Input input);

float spectral_rolloff(Env& env, Input input, float rolloff);

/* ------------------------------------- Algorithm interface ------------------------------------ */

enum class ParameterType {
    Unknown,
    Boolean,
    Int32,
    UInt32,
    Float,
};

struct ParameterDescriptor {
    ParameterType type;
    const char* name;
    const char* description;
    const char* unit;
};

struct AlgorithmInfo {
    const char* identifier;
    std::span<const ParameterDescriptor> parameters;
};

class Algorithm {
public:
    virtual ~Algorithm() = default;

    virtual AlgorithmInfo info() const noexcept = 0;

    virtual bool set_parameter(const char* name, double value) noexcept = 0;
    virtual std::optional<double> get_parameter(const char* name) const noexcept = 0;

    virtual double process(Env& env, Input input) = 0;
};

std::unique_ptr<Algorithm> make_algorithm(const char* identifier);

}  // namespace openae::features
