#include "vamp.h"

#include "openae/common.hpp"
#include "openae/features.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <array>
#include <complex>
#include <cstring>
#include <span>
#include <vector>

namespace {

constexpr unsigned int host_api_version = VAMP_API_VERSION;
constexpr float samplerate = 1000.0F;
constexpr unsigned int block_size = 128;
constexpr unsigned int bins = (block_size / 2) + 1;

const VampPluginDescriptor* find_plugin(const char* identifier) {
    for (unsigned int i = 0;; ++i) {
        const auto* d = vampGetPluginDescriptor(host_api_version, i);
        if (d == nullptr) {
            return nullptr;
        }
        if (std::strcmp(d->identifier, identifier) == 0) {
            return d;
        }
    }
}

unsigned int plugin_count() {
    unsigned int n = 0;
    while (vampGetPluginDescriptor(host_api_version, n) != nullptr) {
        ++n;
    }
    return n;
}

// Process a single block and return the single output value.
float run_plugin(const VampPluginDescriptor* d, VampPluginHandle handle, const float* block) {
    const std::array inputs = {block};
    auto* features = d->process(handle, inputs.data(), 0, 0);
    REQUIRE(features != nullptr);
    REQUIRE(features->featureCount == 1);
    REQUIRE(features->features[0].v1.valueCount == 1);
    const float value = features->features[0].v1.values[0];
    d->releaseFeatureSet(features);
    return value;
}

// Complex bins -> interleaved real/imag pairs (Vamp host format).
std::vector<float> interleave(std::span<const std::complex<float>> spectrum) {
    std::vector<float> buffer;
    buffer.reserve(2 * spectrum.size());
    for (const auto& c : spectrum) {
        buffer.push_back(c.real());
        buffer.push_back(c.imag());
    }
    return buffer;
}

}  // namespace

TEST_CASE("vampGetPluginDescriptor rejects unsupported host API version", "[vamp]") {
    CHECK(vampGetPluginDescriptor(0, 0) == nullptr);
}

TEST_CASE("vampGetPluginDescriptor returns null past the last plugin", "[vamp]") {
    const auto n = plugin_count();
    REQUIRE(n > 0);
    CHECK(vampGetPluginDescriptor(host_api_version, n) == nullptr);
    CHECK(vampGetPluginDescriptor(host_api_version, n + 100) == nullptr);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity), inflated by assertion macros
TEST_CASE("every plugin descriptor has well-formed metadata", "[vamp]") {
    const auto n = plugin_count();
    for (unsigned int i = 0; i < n; ++i) {
        const auto* d = vampGetPluginDescriptor(host_api_version, i);
        REQUIRE(d != nullptr);
        CAPTURE(i, d->identifier);
        // Descriptors declare API version 1 (no V2-only features used).
        CHECK(d->vampApiVersion == 1);
        CHECK(d->identifier != nullptr);
        CHECK(d->name != nullptr);
        CHECK(d->description != nullptr);
        CHECK(d->maker != nullptr);
        CHECK(std::strlen(d->identifier) > 0);
        CHECK(std::strlen(d->name) > 0);
        CHECK((d->inputDomain == vampTimeDomain || d->inputDomain == vampFrequencyDomain));
        for (unsigned int p = 0; p < d->parameterCount; ++p) {
            REQUIRE(d->parameters[p] != nullptr);
            CHECK(d->parameters[p]->identifier != nullptr);
        }
    }
}

// The algorithms are covered by the library tests. Here, expected values come from direct library
// calls, so a mismatch can only be a marshalling error in the adapter, not an algorithm change.

TEST_CASE("process() feeds time-domain input to the feature and returns its value", "[vamp]") {
    const auto* d = find_plugin("rms");
    REQUIRE(d != nullptr);
    REQUIRE(d->inputDomain == vampTimeDomain);

    // Varying signal so marshalling errors change the result.
    std::vector<float> block(block_size);
    for (unsigned int i = 0; i < block_size; ++i) {
        block[i] = static_cast<float>(i % 17) - 8.0F;
    }

    openae::Env env{};
    const auto expected = openae::features::rms(
        env,
        {
            .samplerate = samplerate,
            .timedata = block,
            .spectrum = {},
            .fingerprint = {},
        }
    );

    auto* handle = d->instantiate(d, samplerate);
    REQUIRE(handle != nullptr);
    REQUIRE(d->initialise(handle, 1, block_size, block_size) == 1);
    CHECK(d->getOutputCount(handle) == 1);
    CHECK_THAT(run_plugin(d, handle, block.data()), Catch::Matchers::WithinRel(expected, 1e-6F));
    d->cleanup(handle);
}

TEST_CASE("process() maps frequency-domain input to a one-sided complex spectrum", "[vamp]") {
    const auto* d = find_plugin("spectral-peak-frequency");
    REQUIRE(d != nullptr);
    REQUIRE(d->inputDomain == vampFrequencyDomain);

    // Hot interior bin on a varying background: bin count or layout errors change the result.
    std::vector<std::complex<float>> spectrum(bins);
    for (unsigned int b = 0; b < bins; ++b) {
        spectrum[b] = {0.01F * static_cast<float>(b), 0.02F * static_cast<float>(b)};
    }
    spectrum[16] = {10.0F, 0.0F};
    const auto block = interleave(spectrum);

    openae::Env env{};
    const auto expected = openae::features::spectral_peak_frequency(
        env,
        {
            .samplerate = samplerate,
            .timedata = {},
            .spectrum = spectrum,
            .fingerprint = {},
        }
    );

    auto* handle = d->instantiate(d, samplerate);
    REQUIRE(handle != nullptr);
    REQUIRE(d->initialise(handle, 1, block_size, block_size) == 1);
    CHECK_THAT(run_plugin(d, handle, block.data()), Catch::Matchers::WithinRel(expected, 1e-6F));
    d->cleanup(handle);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity), inflated by assertion macros
TEST_CASE("parameters round-trip and are forwarded to the feature computation", "[vamp]") {
    const auto* d = find_plugin("partial-power");
    REQUIRE(d != nullptr);
    REQUIRE(d->inputDomain == vampFrequencyDomain);
    REQUIRE(d->parameterCount == 2);

    auto* handle = d->instantiate(d, samplerate);
    REQUIRE(handle != nullptr);

    SECTION("defaults and get/set round-trip") {
        CHECK(d->getParameter(handle, 0) == d->parameters[0]->defaultValue);
        CHECK(d->getParameter(handle, 1) == d->parameters[1]->defaultValue);

        d->setParameter(handle, 0, 100.0F);
        CHECK(d->getParameter(handle, 0) == 100.0F);

        // Out-of-range indices: get returns 0, set is a no-op.
        CHECK(d->getParameter(handle, -1) == 0.0F);
        CHECK(d->getParameter(handle, 2) == 0.0F);
        d->setParameter(handle, -1, 1.0F);
        d->setParameter(handle, 2, 1.0F);
        CHECK(d->getParameter(handle, 0) == 100.0F);
    }

    SECTION("parameter values are forwarded to the computation") {
        constexpr float fmin = 50.0F;
        constexpr float fmax = 200.0F;

        std::vector<std::complex<float>> spectrum(bins);
        for (unsigned int b = 0; b < bins; ++b) {
            spectrum[b] = {static_cast<float>(b), 0.0F};
        }
        const auto block = interleave(spectrum);

        openae::Env env{};
        const openae::features::Input input{
            .samplerate = samplerate,
            .timedata = {},
            .spectrum = spectrum,
            .fingerprint = {},
        };
        const auto expected = openae::features::partial_power(env, input, fmin, fmax);
        // Must differ from the default-parameter result, or dropped parameters go unnoticed.
        REQUIRE(
            expected
            != openae::features::partial_power(
                env, input, d->parameters[0]->defaultValue, d->parameters[1]->defaultValue
            )
        );

        d->setParameter(handle, 0, fmin);
        d->setParameter(handle, 1, fmax);
        REQUIRE(d->initialise(handle, 1, block_size, block_size) == 1);
        CHECK_THAT(
            run_plugin(d, handle, block.data()), Catch::Matchers::WithinRel(expected, 1e-6F)
        );
    }

    d->cleanup(handle);
}

TEST_CASE("rejecting non-mono channel counts in initialise", "[vamp]") {
    const auto* d = find_plugin("rms");
    REQUIRE(d != nullptr);
    auto* handle = d->instantiate(d, samplerate);
    REQUIRE(handle != nullptr);
    CHECK(d->initialise(handle, 2, block_size, block_size) == 0);
    d->cleanup(handle);
}
