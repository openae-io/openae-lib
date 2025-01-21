#include <filesystem>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <toml++/toml.hpp>

#include "openae/common.hpp"
#include "openae/features.hpp"

#include "test_config.hpp"

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

template <typename T>
static std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec.at(i);
        if (i + 1 < vec.size()) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

static std::ostream& operator<<(std::ostream& os, const OwningInput& input) {
    os << "{";
    os << "samplerate: " << input.samplerate << ", ";
    os << "timedata: " << input.timedata << ", ";
    os << "spectrum: " << input.spectrum;
    os << "}";
    return os;
}

struct TestCase {
    std::string name;
    OwningInput input;
    double output;
};

struct TestFile {
    std::string feature;
    std::vector<TestCase> tests;
};

class ParseError : public std::runtime_error {
    using runtime_error::runtime_error;
};

template <typename T>
static std::vector<T> parse_array(const toml::node& node) {
    if (const auto* arr = node.as_array()) {
        std::vector<T> vec(arr->size());
        std::transform(arr->begin(), arr->end(), vec.begin(), [](const toml::node& v) {
            return v.value<T>().value();
        });
        return vec;
    }
    throw ParseError("node is not an array");
}

static OwningInput parse_input(const toml::node& node) {
    if (const auto* tbl = node.as_table()) {
        return OwningInput{
            .samplerate = tbl->at_path("samplerate").value_or(0.0f),
            .timedata = parse_array<float>(tbl->at("timedata")),
            .spectrum = {},  // TODO
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

static TestFile parse_test_file(std::string_view filename) {
    const auto path = std::filesystem::path{openae::test::test_dir} / filename;
    const toml::table tbl = toml::parse_file(path.c_str());
    return TestFile{
        .feature = tbl.at("feature").value<std::string>().value(),
        .tests = parse_test_cases(tbl.at("tests")),
    };
}

TEST_CASE("Features") {
    openae::Env env{};

    for (const auto& test : parse_test_file("test_features_rms.toml").tests) {
        DYNAMIC_SECTION(test.name) {
            CAPTURE(test.input);
            CHECK(openae::features::rms(env, test.input) == test.output);
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
