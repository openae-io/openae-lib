#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>
#include <toml++/toml.hpp>

#include "openae/features.hpp"

#include "test_config.hpp"

struct OwningInput {
    float samplerate;
    std::vector<float> timedata;
    std::vector<std::complex<float>> spectrum;

    operator openae::features::Input() const {
        return {
            .samplerate = samplerate,
            .timedata = timedata,
            .spectrum = spectrum,
        };
    }
};

static OwningInput timedata_input(std::vector<float> timedata, float samplerate = 0.0f) {
    return OwningInput{
        .samplerate = samplerate,
        .timedata = std::move(timedata),
        .spectrum = {}
    };
}

struct TestCase {
    std::string name;
    OwningInput input;
    double output;
};

struct FeatureTests {
    std::string feature;
    std::vector<TestCase> tests;
};

class ParseError : public std::runtime_error {
    using runtime_error::runtime_error;
};

static std::vector<float> parse_input_timedata(const toml::node& node) {
    std::vector<float> vec;
    if (const auto* arr = node.as_array()) {
        for (const auto& value : *arr) {
            if (value.is_number()) {
                vec.push_back(value.value_or<float>(0.0f));
            } else {
                ParseError("input.timedata invalid (array with ints/floats required)");
            }
        }
        return vec;
    }
    throw ParseError("input.timedata is not an array");
}

static OwningInput parse_input(const toml::node& node) {
    if (const auto* tbl = node.as_table()) {
        return OwningInput{
            .samplerate = (*tbl)["samplerate"].value_or(0.0f),
            .timedata = parse_input_timedata(tbl->at("timedata")),

        };
    }
    throw ParseError("input is not a table");
}

static TestCase parse_test_case(const toml::node& node) {
    if (const toml::table* tbl = node.as_table()) {
        return TestCase{
            .name = tbl->at("name").value<std::string>().value(),
            .input = parse_input(tbl->at("input")),
            .output = tbl->at("output").value<double>().value(),
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

static FeatureTests parse_feature_tests(std::string_view filename) {
    const auto path = std::filesystem::path{openae::test::test_dir} / filename;
    const toml::table tbl = toml::parse_file(path.c_str());
    return FeatureTests{
        .feature = tbl.at("feature").value<std::string>().value(),
        .tests = parse_test_cases(tbl.at("tests")),
    };
}

TEST_CASE("Features") {
    const auto test_file = parse_feature_tests("test_features_rms.toml");
    openae::Env env{};

    CAPTURE(test_file.feature);
    for (const auto& test : test_file.tests) {
        DYNAMIC_SECTION(test.name) {
            CAPTURE(test.input.samplerate);
            CAPTURE(test.input.timedata);
            CAPTURE(test.input.spectrum);
            CHECK(openae::features::rms(env, test.input) == test.output);
        }
    }
}

TEST_CASE("Algorithm") {
    openae::Env env{};

    SECTION("No parameter") {
        auto algorithm = openae::features::make_algorithm("rms");
        auto input = timedata_input({1, -1}, 2.0f);
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
