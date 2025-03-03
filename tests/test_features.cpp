#include <algorithm>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <toml++/toml.hpp>

#include "openae/common.hpp"
#include "openae/features.hpp"

#include "test_config.hpp"
#include "tostring.hpp"

struct OwningInput {
    float samplerate;
    std::vector<float> timedata;
    std::vector<std::complex<float>> spectrum;

    operator openae::features::Input() const {
        return {
            .samplerate = samplerate,
            .timedata = timedata,
            .spectrum = spectrum,
            .fingerprint = {},
        };
    }
};

using ParameterMap = std::map<std::string, double>;

struct TestCase {
    std::string name;
    OwningInput input;
    ParameterMap parameters;
    double result;
};

struct TestConfig {
    std::string feature;
    std::vector<TestCase> tests;
};

class ParseError : public std::runtime_error {
    using runtime_error::runtime_error;
};

template <typename T, typename TomlType = T>
std::vector<T> parse_array(const toml::node& node) {
    if (const auto* arr = node.as_array()) {
        std::vector<T> vec(arr->size());
        std::transform(arr->begin(), arr->end(), vec.begin(), [](const toml::node& v) {
            return v.value<TomlType>().value();
        });
        return vec;
    }
    throw ParseError("node is not an array");
}

OwningInput parse_input(const toml::node& node) {
    if (const auto* tbl = node.as_table()) {
        return OwningInput{
            .samplerate = tbl->at_path("samplerate").value_or(0.0f),
            .timedata = tbl->contains("timedata")
                ? parse_array<float>(tbl->at("timedata"))
                : std::vector<float>{},
            .spectrum = tbl->contains("spectrum")
                ? parse_array<std::complex<float>, float>(tbl->at("spectrum"))
                : std::vector<std::complex<float>>{},
        };
    }
    throw ParseError("input is not a table");
}

ParameterMap parse_parameters(const toml::node& node) {
    if (const auto* tbl = node.as_table()) {
        ParameterMap params;
        for (const auto& [key, value] : *tbl) {
            params.emplace(key.str(), value.value<double>().value());
        }
        return params;
    }
    throw ParseError("params is not a table");
}

TestCase parse_test_case(const toml::node& node) {
    if (const toml::table* tbl = node.as_table()) {
        return TestCase{
            .name = tbl->at("name").value<std::string>().value(),
            .input = parse_input(tbl->at("input")),
            .parameters = tbl->contains("params")
                ? parse_parameters(tbl->at("params"))
                : ParameterMap{},
            .result = tbl->at("result").value<double>().value(),
        };
    }
    throw ParseError("test is not an object");
}

std::vector<TestCase> parse_test_cases(const toml::node& node) {
    std::vector<TestCase> tests;
    if (const toml::array* arr = node.as_array()) {
        for (const auto& test : *arr) {
            tests.push_back(parse_test_case(test));
        }
    }
    return tests;
}

TestConfig parse_test_config(std::filesystem::path path) {
    const toml::table tbl = toml::parse_file(path.c_str());
    return TestConfig{
        .feature = tbl.at("feature").value<std::string>().value(),
        .tests = parse_test_cases(tbl.at("tests")),
    };
}

template <typename Tuple, typename F>
constexpr void for_each(Tuple&& tuple, F&& f) {
    std::apply(
        [&f](auto&&... args) { (std::invoke(f, std::forward<decltype(args)>(args)), ...); },
        std::forward<Tuple>(tuple)
    );
}

template <typename R, typename... Args>
R compute_feature(
    R (*func)(openae::Env&, openae::features::Input, Args...),
    openae::features::Input input,
    const std::array<std::string, sizeof...(Args)>& param_names,
    const ParameterMap& param_values
) {
    openae::Env env{};
    std::tuple<Args...> args;
    for_each(args, [&, index = size_t{0}]<typename Arg>(Arg& arg) mutable {
        const auto param_name = param_names[index++];
        if (not param_values.contains(param_name)) {
            throw std::runtime_error(std::format("Parameter not found: {}", param_name));
        }
        arg = static_cast<Arg>(param_values.at(param_name));
    });

    return std::apply(
        [&](auto&&... args_unpacked) {
            return func(env, input, std::forward<decltype(args_unpacked)>(args_unpacked)...);
        },
        args
    );
}

template <typename Intermediate, typename T>
constexpr T cast_error(T value) {
    return std::abs(value - static_cast<T>(static_cast<Intermediate>(value)));
}

template <typename R, typename... Args>
void run_tests(
    std::string_view test_config_filename,
    R (*func)(openae::Env&, openae::features::Input, Args...),
    const std::array<std::string, sizeof...(Args)>& param_names
) {
    const auto test_config = parse_test_config(
        std::filesystem::path{openae::test::test_dir} / test_config_filename
    );
    for (const auto& test : test_config.tests) {
        DYNAMIC_SECTION(test.name) {
            CAPTURE(test.name);
            CAPTURE(test.input.samplerate);
            CAPTURE(test.input.timedata);
            CAPTURE(test.input.spectrum);
            CAPTURE(test.parameters);

            REQUIRE(test.parameters.size() == param_names.size());

            auto result = compute_feature(func, test.input, param_names, test.parameters);
            CAPTURE(test.result, result);

            if (std::isnan(test.result)) {
                REQUIRE(std::isnan(result));
            } else if (std::isinf(test.result)) {
                REQUIRE(std::isinf(result));
            } else {
                const Catch::Matchers::WithinRelMatcher tol_rel{test.result, 1e-6};
                const Catch::Matchers::WithinAbsMatcher tol_abs{
                    test.result, cast_error<float>(test.result)
                };
                REQUIRE_THAT(result, tol_rel || tol_abs);
            }
        }
    }
}

#define TEST_CASE_FEATURE(name, func, ...)                                                              \
    TEST_CASE("Feature " name) {                                                                   \
        run_tests("test_features_" name ".toml", func, {__VA_ARGS__});                             \
    }

TEST_CASE_FEATURE("clearance-factor", openae::features::clearance_factor)
TEST_CASE_FEATURE("crest-factor", openae::features::crest_factor)
TEST_CASE_FEATURE("energy", openae::features::energy)
TEST_CASE_FEATURE("impulse-factor", openae::features::impulse_factor)
TEST_CASE_FEATURE("kurtosis", openae::features::kurtosis)
TEST_CASE_FEATURE("partial-power", openae::features::partial_power, "fmin", "fmax")
TEST_CASE_FEATURE("peak-amplitude", openae::features::peak_amplitude)
TEST_CASE_FEATURE("rms", openae::features::rms)
TEST_CASE_FEATURE("shape-factor", openae::features::shape_factor)
TEST_CASE_FEATURE("skewness", openae::features::skewness)
TEST_CASE_FEATURE("spectral-centroid", openae::features::spectral_centroid)
TEST_CASE_FEATURE("spectral-flatness", openae::features::spectral_flatness)
TEST_CASE_FEATURE("spectral-kurtosis", openae::features::spectral_kurtosis)
TEST_CASE_FEATURE("spectral-peak-frequency", openae::features::spectral_peak_frequency)
TEST_CASE_FEATURE("spectral-rolloff", openae::features::spectral_rolloff, "rolloff")
TEST_CASE_FEATURE("spectral-skewness", openae::features::spectral_skewness)
TEST_CASE_FEATURE("spectral-variance", openae::features::spectral_variance)
TEST_CASE_FEATURE("zero-crossing-rate", openae::features::zero_crossing_rate)
