#include <filesystem>
#include <ostream>
#include <stdexcept>
#include <string>
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
        return {.samplerate = samplerate, .timedata = timedata, .spectrum = spectrum};
    }
};

static OwningInput make_input_timedata(std::vector<float> timedata, float samplerate = 0.0f) {
    return {.samplerate = samplerate, .timedata = std::move(timedata), .spectrum = {}};
}

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
static std::vector<T> parse_array(const toml::node& node) {
    if (const auto* arr = node.as_array()) {
        std::vector<T> vec(arr->size());
        std::transform(arr->begin(), arr->end(), vec.begin(), [](const toml::node& v) {
            return v.value<TomlType>().value();
        });
        return vec;
    }
    throw ParseError("node is not an array");
}

static OwningInput parse_input(const toml::node& node) {
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

static ParameterMap parse_parameters(const toml::node& node) {
    if (const auto* tbl = node.as_table()) {
        ParameterMap params;
        for (const auto& [key, value] : *tbl) {
            params.emplace(key.str(), value.value<double>().value());
        }
        return params;
    }
    throw ParseError("params is not a table");
}

static TestCase parse_test_case(const toml::node& node) {
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

static std::vector<TestCase> parse_test_cases(const toml::node& node) {
    std::vector<TestCase> tests;
    if (const toml::array* arr = node.as_array()) {
        for (const auto& test : *arr) {
            tests.push_back(parse_test_case(test));
        }
    }
    return tests;
}

static TestConfig parse_test_config(std::filesystem::path path) {
    const toml::table tbl = toml::parse_file(path.c_str());
    return TestConfig{
        .feature = tbl.at("feature").value<std::string>().value(),
        .tests = parse_test_cases(tbl.at("tests")),
    };
}

static std::vector<std::filesystem::path> find_test_configs() {
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(openae::test::test_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".toml") {
            files.push_back(entry.path());
        }
    }
    return files;
}

template <typename Intermediate, typename T>
static T cast_error(T value) {
    return std::abs(value - static_cast<T>(static_cast<Intermediate>(value)));
}

TEST_CASE("Features") {
    for (const auto& path : find_test_configs()) {
        CAPTURE(path.filename());
        const auto test_config = parse_test_config(path);
        DYNAMIC_SECTION(test_config.feature) {
            for (const auto& test : test_config.tests) {
                DYNAMIC_SECTION(test.name) {
                    CAPTURE(test.name);
                    CAPTURE(test.input.samplerate);
                    CAPTURE(test.input.timedata);
                    CAPTURE(test.input.spectrum);
                    CAPTURE(test.parameters);

                    auto algorithm = openae::features::make_algorithm(test_config.feature.c_str());
                    REQUIRE(algorithm != nullptr);

                    for (const auto& [name, value] : test.parameters) {
                        CAPTURE(name, value);
                        REQUIRE(algorithm->set_parameter(name.c_str(), value));
                        REQUIRE_THAT(
                            algorithm->get_parameter(name.c_str()).value(),
                            Catch::Matchers::WithinAbs(value, cast_error<float>(value))
                        );
                    }

                    openae::Env env{};
                    const auto result = algorithm->process(env, test.input);
                    CAPTURE(test.result, result);

                    if (std::isnan(test.result)) {
                        REQUIRE(std::isnan(result));
                    } else if (std::isinf(test.result)) {
                        REQUIRE(std::isinf(result));
                    } else {
                        REQUIRE_THAT(
                            result,
                            Catch::Matchers::WithinAbs(
                                test.result, cast_error<float>(test.result)
                            ) || Catch::Matchers::WithinRel(test.result, 1e-6)
                        );
                    }
                }
            }
        }
    }
}

TEST_CASE("Algorithm") {
    openae::Env env{};

    SECTION("No parameter") {
        auto algorithm = openae::features::make_algorithm("rms");
        auto input = make_input_timedata({1, -1}, 2.0f);
        CHECK(algorithm->process(env, input) == 1.0f);
    }

    SECTION("With parameters") {
        auto algorithm = openae::features::make_algorithm("spectral-rolloff");
        CHECK(algorithm->get_parameter("rolloff").has_value());
        CHECK(algorithm->get_parameter("rolloff").value() == 0.0);
        CHECK(algorithm->set_parameter("rolloff", 0.9f));
        CHECK(algorithm->get_parameter("rolloff").value() == 0.9f);

        CHECK_FALSE(algorithm->get_parameter("invalid").has_value());
        CHECK_FALSE(algorithm->set_parameter("invalid", 11.11f));
    }
}
