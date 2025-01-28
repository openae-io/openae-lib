#pragma once

#include <array>
#include <tuple>

#include "openae/common.hpp"
#include "openae/features.hpp"

#include "utils.hpp"

namespace openae::features {

struct Parameter {
    const char* name;
};

template <typename R, typename... Args>
struct Feature {
    using ParameterTuple = std::tuple<Args...>;

    R (*func)(Env&, Input, Args...);
    const char* identifier;
    std::array<RequireInit<Parameter>, sizeof...(Args)> parameters;
};

consteval auto meta() {
    return std::tuple{
        Feature{&peak_amplitude, "peak-amplitude", {}},
        Feature{&energy, "energy", {}},
        Feature{&rms, "rms", {}},
        Feature{&crest_factor, "crest-factor", {}},
        Feature{&impulse_factor, "impulse-factor", {}},
        Feature{&clearance_factor, "clearance-factor", {}},
        Feature{&shape_factor, "shape-factor", {}},
        Feature{&skewness, "skewness", {}},
        Feature{&kurtosis, "kurtosis", {}},
        Feature{&zero_crossing_rate, "zero-crossing-rate", {}},
        Feature{&partial_power, "partial-power", {Parameter{"fmin"}, Parameter{"fmax"}}},
        Feature{&spectral_peak_frequency, "spectral-peak-frequency", {}},
        Feature{&spectral_centroid, "spectral-centroid", {}},
        Feature{&spectral_variance, "spectral-variance", {}},
        Feature{&spectral_skewness, "spectral-skewness", {}},
        Feature{&spectral_rolloff, "spectral-rolloff", {Parameter{"rolloff"}}},
    };
}

}  // namespace openae::features
