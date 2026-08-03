#include "vamp.h"

#include "openae/common.hpp"
#include "openae/features.hpp"

#include <algorithm>
#include <array>
#include <complex>
#include <cstddef>
#include <memory>
#include <span>
#include <utility>

namespace {

using openae::Env;
using openae::features::Input;

enum class Domain : unsigned char { Time, Frequency };

struct PluginSpec {
    const char* identifier = nullptr;
    const char* name = nullptr;
    const char* description = nullptr;
    const char* unit = "";
    Domain domain = Domain::Time;
    std::span<const VampParameterDescriptor* const> parameters;
    float (*compute)(Env& env, Input input, std::span<const float> parameters) = nullptr;
};

inline constexpr VampParameterDescriptor param_fmin{
    .identifier = "fmin",
    .name = "Lower frequency",
    .description = "Lower frequency bound (clamped to Nyquist by the plugin).",
    .unit = "Hz",
    .minValue = 0.0F,
    .maxValue = 1.0e6F,
    .defaultValue = 0.0F,
    .isQuantized = 0,
    .quantizeStep = 0.0F,
    .valueNames = nullptr,
};
inline constexpr VampParameterDescriptor param_fmax{
    .identifier = "fmax",
    .name = "Upper frequency",
    .description = "Upper frequency bound (clamped to Nyquist by the plugin).",
    .unit = "Hz",
    .minValue = 0.0F,
    .maxValue = 1.0e6F,
    .defaultValue = 1.0e6F,
    .isQuantized = 0,
    .quantizeStep = 0.0F,
    .valueNames = nullptr,
};
inline constexpr VampParameterDescriptor param_rolloff{
    .identifier = "rolloff",
    .name = "Rolloff fraction",
    .description = "Fraction of total spectral energy below the rolloff frequency (range [0, 1]).",
    .unit = "",
    .minValue = 0.0F,
    .maxValue = 1.0F,
    .defaultValue = 0.85F,
    .isQuantized = 0,
    .quantizeStep = 0.0F,
    .valueNames = nullptr,
};

inline constexpr std::array params_partial_power = {&param_fmin, &param_fmax};
inline constexpr std::array params_rolloff = {&param_rolloff};

inline constexpr std::array specs{
    // Time-domain features
    PluginSpec{
        .identifier = "peak-amplitude",
        .name = "Peak amplitude",
        .description = "The maximum absolute amplitude of a signal.",
        .unit = "V",
        .domain = Domain::Time,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::peak_amplitude(env, input);
        },
    },
    PluginSpec{
        .identifier = "energy",
        .name = "Energy",
        .description = "The integral of the signal's squared values over time.",
        .unit = "V^2s",
        .domain = Domain::Time,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::energy(env, input);
        },
    },
    PluginSpec{
        .identifier = "rms",
        .name = "RMS",
        .description = "A measure for the average energy of a signal.",
        .unit = "V",
        .domain = Domain::Time,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::rms(env, input);
        },
    },
    PluginSpec{
        .identifier = "crest-factor",
        .name = "Crest factor",
        .description = "The ratio of the peak amplitude to the RMS of a signal.",
        .unit = "",
        .domain = Domain::Time,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::crest_factor(env, input);
        },
    },
    PluginSpec{
        .identifier = "impulse-factor",
        .name = "Impulse factor",
        .description = "The ratio of peak amplitude and mean of absolute values.",
        .unit = "",
        .domain = Domain::Time,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::impulse_factor(env, input);
        },
    },
    PluginSpec{
        .identifier = "clearance-factor",
        .name = "Clearance factor",
        .description = "The ratio of the peak amplitude and the squared mean of the square roots of the absolute amplitudes.",
        .unit = "",
        .domain = Domain::Time,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::clearance_factor(env, input);
        },
    },
    PluginSpec{
        .identifier = "shape-factor",
        .name = "Shape factor",
        .description = "The ratio of the RMS value and the mean of absolute values.",
        .unit = "",
        .domain = Domain::Time,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::shape_factor(env, input);
        },
    },
    PluginSpec{
        .identifier = "skewness",
        .name = "Skewness",
        .description = "A statistical measure that quantifies the asymmetry of a dataset's probability distribution.",
        .unit = "",
        .domain = Domain::Time,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::skewness(env, input);
        },
    },
    PluginSpec{
        .identifier = "kurtosis",
        .name = "Kurtosis",
        .description = "A statistical measure that describes the shape of a distribution, focusing on its tails.",
        .unit = "",
        .domain = Domain::Time,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::kurtosis(env, input);
        },
    },
    PluginSpec{
        .identifier = "zero-crossing-rate",
        .name = "Zero-crossing rate",
        .description = "The rate at which a signal changes from positive to zero to negative or vice versa.",
        .unit = "Hz",
        .domain = Domain::Time,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::zero_crossing_rate(env, input);
        },
    },

    // Frequency-domain features
    PluginSpec{
        .identifier = "partial-power",
        .name = "Partial power",
        .description = "The proportion of energy within a specified frequency band [fmin, fmax) relative to total energy.",
        .unit = "",
        .domain = Domain::Frequency,
        .parameters = params_partial_power,
        .compute = [](Env& env, Input input, std::span<const float> parameters) {
            return openae::features::partial_power(env, input, parameters[0], parameters[1]);
        },
    },
    PluginSpec{
        .identifier = "spectral-peak-frequency",
        .name = "Spectral peak frequency",
        .description = "The frequency at which a signal has its highest energy.",
        .unit = "Hz",
        .domain = Domain::Frequency,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::spectral_peak_frequency(env, input);
        },
    },
    PluginSpec{
        .identifier = "spectral-centroid",
        .name = "Spectral centroid",
        .description = "The centroid frequency indicates where the center of mass of the spectrum is located.",
        .unit = "Hz",
        .domain = Domain::Frequency,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::spectral_centroid(env, input);
        },
    },
    PluginSpec{
        .identifier = "spectral-variance",
        .name = "Spectral variance",
        .description = "Quantifies the dispersion of the spectral content around the spectral centroid.",
        .unit = "Hz^2",
        .domain = Domain::Frequency,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::spectral_variance(env, input);
        },
    },
    PluginSpec{
        .identifier = "spectral-skewness",
        .name = "Spectral skewness",
        .description = "A measure of the asymmetry of the power spectrum around its mean (the spectral centroid).",
        .unit = "",
        .domain = Domain::Frequency,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::spectral_skewness(env, input);
        },
    },
    PluginSpec{
        .identifier = "spectral-kurtosis",
        .name = "Spectral kurtosis",
        .description = "A measure of the tailedness or peakedness of the power spectrum around its mean.",
        .unit = "",
        .domain = Domain::Frequency,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::spectral_kurtosis(env, input);
        },
    },
    PluginSpec{
        .identifier = "spectral-rolloff",
        .name = "Spectral rolloff",
        .description = "The frequency below which a specified proportion of the total spectral energy is contained.",
        .unit = "Hz",
        .domain = Domain::Frequency,
        .parameters = params_rolloff,
        .compute = [](Env& env, Input input, std::span<const float> parameters) {
            return openae::features::spectral_rolloff(env, input, parameters[0]);
        },
    },
    PluginSpec{
        .identifier = "spectral-entropy",
        .name = "Spectral entropy",
        .description = "Spectral peakedness: sharp peaks yield low entropy, flat spectra yield high entropy.",
        .unit = "",
        .domain = Domain::Frequency,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::spectral_entropy(env, input);
        },
    },
    PluginSpec{
        .identifier = "spectral-flatness",
        .name = "Spectral flatness",
        .description = "A measure used to quantify how flat or \"white\" a signal's power spectrum is.",
        .unit = "",
        .domain = Domain::Frequency,
        .parameters = {},
        .compute = [](Env& env, Input input, [[maybe_unused]] std::span<const float> parameters) {
            return openae::features::spectral_flatness(env, input);
        },
    },
};

constexpr std::size_t maxParameters = 2;
static_assert(
    std::ranges::all_of(
        specs, [](const PluginSpec& s) { return s.parameters.size() <= maxParameters; }
    ),
    "a PluginSpec has more parameters than Instance::params can hold"
);

struct Instance {
    const PluginSpec* spec;
    float samplerate;
    unsigned int block_size = 0;
    std::array<float, maxParameters> parameters{};

    // Execution context with cache, reused across process() calls.
    std::unique_ptr<openae::Cache, void (*)(openae::Cache*)> cache = openae::make_cache();
    Env env{};

    // Output value; featureUnion.v1.values points here.
    float value = 0.0F;

    // Single-value API version 1 feature list, wired up in the constructor.
    VampFeatureUnion featureUnion{};
    VampFeatureList featureList{};

    Instance(const PluginSpec* s, float sr)
        : spec(s),
          samplerate(sr) {
        env.cache = cache.get();
        for (std::size_t i = 0; i < s->parameters.size() && i < maxParameters; ++i) {
            parameters[i] = s->parameters[i]->defaultValue;
        }
        featureUnion.v1.valueCount = 1;
        featureUnion.v1.values = &value;
        featureList.featureCount = 1;
        featureList.features = &featureUnion;
    }

    // featureUnion.v1.values points into this object; copying/moving would dangle.
    Instance(const Instance&) = delete;
    Instance(Instance&&) = delete;
    Instance& operator=(const Instance&) = delete;
    Instance& operator=(Instance&&) = delete;
    ~Instance() = default;
};

inline VampFeatureList& empty_feature_list() {
    static VampFeatureList empty{0, nullptr};
    return empty;
}

template <std::size_t I>
inline constexpr VampOutputDescriptor output_descriptor{
    .identifier = specs[I].identifier,
    .name = specs[I].name,
    .description = specs[I].description,
    .unit = specs[I].unit,
    .hasFixedBinCount = 1,
    .binCount = 1,
    .binNames = nullptr,
    .hasKnownExtents = 0,
    .minValue = 0.0F,
    .maxValue = 0.0F,
    .isQuantized = 0,
    .quantizeStep = 0.0F,
    .sampleType = vampOneSamplePerStep,
    .sampleRate = 0.0F,
    .hasDuration = 0,
};

template <std::size_t I>
constexpr VampPluginDescriptor make_descriptor() {
    constexpr const PluginSpec& spec = specs[I];
    VampPluginDescriptor d{};
    // No V2-only features used (no durations), so declare API version 1 for any-host compatibility.
    d.vampApiVersion = 1;
    d.identifier = spec.identifier;
    d.name = spec.name;
    d.description = spec.description;
    d.maker = "OpenAE (https://openae.io)";
    d.pluginVersion = 1;
    d.copyright = "MIT";
    d.parameterCount = static_cast<unsigned int>(spec.parameters.size());
    d.parameters = const_cast<const VampParameterDescriptor**>(spec.parameters.data());
    d.programCount = 0;
    d.programs = nullptr;
    d.inputDomain = (spec.domain == Domain::Time) ? vampTimeDomain : vampFrequencyDomain;

    d.instantiate = [](const VampPluginDescriptor*, float sr) -> VampPluginHandle {
        // Exceptions must not cross the C ABI into the host.
        try {
            return new Instance(&specs[I], sr);
        } catch (...) {
            return nullptr;
        }
    };
    d.cleanup = [](VampPluginHandle h) { delete static_cast<Instance*>(h); };
    d.initialise =
        [](VampPluginHandle h, unsigned int channels, unsigned int, unsigned int block_size
        ) -> int {
        if (channels != 1) {
            return 0;
        }
        static_cast<Instance*>(h)->block_size = block_size;
        return 1;
    };
    d.reset = [](VampPluginHandle) {};
    d.getParameter = [](VampPluginHandle h, int idx) -> float {
        auto* self = static_cast<Instance*>(h);
        if (idx < 0 || idx >= static_cast<int>(self->spec->parameters.size())) {
            return 0.0F;
        }
        return self->parameters[idx];
    };
    d.setParameter = [](VampPluginHandle h, int idx, float v) {
        auto* self = static_cast<Instance*>(h);
        if (idx < 0 || idx >= static_cast<int>(self->spec->parameters.size())) {
            return;
        }
        self->parameters[idx] = v;
    };
    d.getCurrentProgram = [](VampPluginHandle) -> unsigned int { return 0; };
    d.selectProgram = [](VampPluginHandle, unsigned int) {};
    d.getPreferredStepSize = [](VampPluginHandle) -> unsigned int { return 0; };
    d.getPreferredBlockSize = [](VampPluginHandle) -> unsigned int { return 0; };
    d.getMinChannelCount = [](VampPluginHandle) -> unsigned int { return 1; };
    d.getMaxChannelCount = [](VampPluginHandle) -> unsigned int { return 1; };

    d.getOutputCount = [](VampPluginHandle) -> unsigned int { return 1; };
    d.getOutputDescriptor = [](VampPluginHandle, unsigned int idx) -> VampOutputDescriptor* {
        if (idx != 0) {
            return nullptr;
        }
        // The host treats the descriptor as read-only; releaseOutputDescriptor is a no-op.
        return const_cast<VampOutputDescriptor*>(&output_descriptor<I>);
    };
    d.releaseOutputDescriptor = [](VampOutputDescriptor*) {};

    d.process =
        [](VampPluginHandle h, const float* const* input_buffers, int, int) -> VampFeatureList* {
        auto* self = static_cast<Instance*>(h);
        Input in{};
        in.samplerate = self->samplerate;
        if (self->spec->domain == Domain::Time) {
            in.timedata = std::span<const float>(input_buffers[0], self->block_size);
        } else {
            // Hosts deliver block_size/2 + 1 complex bins as interleaved real/imag floats,
            // matching the guaranteed layout of std::complex<float> ([complex.numbers.general]).
            const std::size_t bins = (self->block_size / 2) + 1;
            in.spectrum = std::span<const std::complex<float>>(
                reinterpret_cast<const std::complex<float>*>(input_buffers[0]), bins
            );
        }
        self->value = self->spec->compute(
            self->env,
            in,
            std::span<const float>(self->parameters.data(), self->spec->parameters.size())
        );
        return &self->featureList;
    };
    d.getRemainingFeatures = [](VampPluginHandle) -> VampFeatureList* {
        return &empty_feature_list();
    };
    d.releaseFeatureSet = [](VampFeatureList*) {};

    return d;
}

template <std::size_t... Is>
constexpr auto make_descriptor_table(std::index_sequence<Is...> /*unused*/) {
    return std::array<VampPluginDescriptor, sizeof...(Is)>{make_descriptor<Is>()...};
}

inline constexpr auto descriptor_table = make_descriptor_table(
    std::make_index_sequence<specs.size()>{}
);

}  // namespace

// Export `vampGetPluginDescriptor` only. MSVC needs the linker pragma: vamp.h declares the function
// without dllexport, and a redeclaration with dllexport would be a linkage mismatch.
#if defined(_MSC_VER)
#define OPENAE_VAMP_EXPORT
#pragma comment(linker, "/EXPORT:vampGetPluginDescriptor")
#else
#define OPENAE_VAMP_EXPORT __attribute__((visibility("default")))
#endif

extern "C" OPENAE_VAMP_EXPORT const VampPluginDescriptor* vampGetPluginDescriptor(
    // NOLINTNEXTLINE(readability-identifier-naming), parameter name dictated by vamp.h declaration
    unsigned int hostApiVersion, unsigned int index
) {
    if (hostApiVersion < 1) {
        return nullptr;
    }
    if (index >= descriptor_table.size()) {
        return nullptr;
    }
    return &descriptor_table[index];
}
