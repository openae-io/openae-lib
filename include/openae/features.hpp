#pragma once

#include <complex>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>

#include "openae/common.hpp"
#include "openae/config.hpp"

namespace openae::features {

/**
 * Represents the input data for feature extraction functions.
 *
 * The Input structure holds both the original signal (`timedata`) and its precomputed `spectrum`.
 * The `spectrum` is typically computed via the discrete Fourier transform (DFT) of the signal,
 * which may be windowed or zero-padded before transformation.
 */
struct Input {
    /// Sampling rate in Hz.
    float samplerate;
    /// Time-domain signal (typically in volts).
    std::span<const float> timedata;
    /// One-sided spectrum of `timedata`.
    std::span<const std::complex<float>> spectrum;
    /// Optional fingerprint for caching.
    std::optional<std::size_t> fingerprint;
};

/// Compute the feature *peak-amplitude*.
/// Definition: https://openae.io/standards/features/latest/peak-amplitude
OPENAE_EXPORT float peak_amplitude(Env& env, Input input);

/// Compute the feature *energy*.
/// Definition: https://openae.io/standards/features/latest/energy
OPENAE_EXPORT float energy(Env& env, Input input);

/// Compute the feature *rms*.
/// Definition: https://openae.io/standards/features/latest/rms
OPENAE_EXPORT float rms(Env& env, Input input);

/// Compute the feature *crest-factor*.
/// Definition: https://openae.io/standards/features/latest/crest-factor
OPENAE_EXPORT float crest_factor(Env& env, Input input);

/// Compute the feature *impulse-factor*.
/// Definition: https://openae.io/standards/features/latest/impulse-factor
OPENAE_EXPORT float impulse_factor(Env& env, Input input);

/// Compute the feature *clearance-factor*.
/// Definition: https://openae.io/standards/features/latest/clearance-factor
OPENAE_EXPORT float clearance_factor(Env& env, Input input);

/// Compute the feature *shape-factor*.
/// Definition: https://openae.io/standards/features/latest/shape-factor
OPENAE_EXPORT float shape_factor(Env& env, Input input);

/// Compute the feature *skewness*.
/// Definition: https://openae.io/standards/features/latest/skewness
OPENAE_EXPORT float skewness(Env& env, Input input);

/// Compute the feature *kurtosis*.
/// Definition: https://openae.io/standards/features/latest/kurtosis
OPENAE_EXPORT float kurtosis(Env& env, Input input);

/// Compute the feature *zero-crossing-rate*.
/// Definition: https://openae.io/standards/features/latest/zero-crossing-rate
OPENAE_EXPORT float zero_crossing_rate(Env& env, Input input);

/// Compute the feature *partial-power*.
/// Definition: https://openae.io/standards/features/latest/partial-power
OPENAE_EXPORT float partial_power(Env& env, Input input, float fmin, float fmax);

/// Compute the feature *spectral-peak-frequency*.
/// Definition: https://openae.io/standards/features/latest/spectral-peak-frequency
OPENAE_EXPORT float spectral_peak_frequency(Env& env, Input input);

/// Compute the feature *spectral-centroid*.
/// Definition: https://openae.io/standards/features/latest/spectral-centroid
OPENAE_EXPORT float spectral_centroid(Env& env, Input input);

/// Compute the feature *spectral-variance*.
/// Definition: https://openae.io/standards/features/latest/spectral-variance
OPENAE_EXPORT float spectral_variance(Env& env, Input input);

/// Compute the feature *spectral-skewness*.
/// Definition: https://openae.io/standards/features/latest/spectral-skewness
OPENAE_EXPORT float spectral_skewness(Env& env, Input input);

/// Compute the feature *spectral-kurtosis*.
/// Definition: https://openae.io/standards/features/latest/spectral-kurtosis
OPENAE_EXPORT float spectral_kurtosis(Env& env, Input input);

/// Compute the feature *spectral-rolloff*.
/// Definition: https://openae.io/standards/features/latest/spectral-rolloff
OPENAE_EXPORT float spectral_rolloff(Env& env, Input input, float rolloff);

/// Compute the feature *spectral-entropy*.
/// Definition: https://openae.io/standards/features/latest/spectral-entropy
OPENAE_EXPORT float spectral_entropy(Env& env, Input input);

/// Compute the feature *spectral-flatness*.
/// Definition: https://openae.io/standards/features/latest/spectral-flatness
OPENAE_EXPORT float spectral_flatness(Env& env, Input input);

}  // namespace openae::features
