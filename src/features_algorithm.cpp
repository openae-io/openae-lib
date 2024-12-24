#include "openae/features.hpp"

#include <type_traits>

#include "features_meta.hpp"
#include "utils.hpp"

namespace openae::features {

template <typename T>
static constexpr ParameterType get_parameter_type() {
    using U = std::remove_cvref_t<T>;
    if constexpr (std::is_same_v<U, bool>) {
        return ParameterType::Boolean;
    } else if constexpr (std::is_same_v<U, int32_t>) {
        return ParameterType::Int32;
    } else if constexpr (std::is_same_v<U, uint32_t>) {
        return ParameterType::UInt32;
    } else if constexpr (std::is_same_v<U, float>) {
        return ParameterType::Float;
    } else {
        return ParameterType::Unknown;
    }
}

template <typename... Args>
static constexpr std::array<ParameterType, sizeof...(Args)> get_parameter_types() {
    return {get_parameter_type<Args>()...};
}

inline constexpr bool equal(const char* lhs, const char* rhs) noexcept {
    return std::string_view{lhs} == std::string_view{rhs};
}

template <typename R, typename... Args>
class AlgorithmAdapter : public Algorithm {
public:
    explicit AlgorithmAdapter(const Feature<R, Args...>& feature) noexcept
        : feature_(&feature) {
        const auto parameter_types = get_parameter_types<Args...>();
        for (size_t i = 0; i < sizeof...(Args); ++i) {
            parameters_[i].type = parameter_types[i];
            parameters_[i].name = feature.parameters[i].value.name;
        }
    }

    AlgorithmInfo info() const noexcept override {
        return AlgorithmInfo{
            .identifier = feature_->identifier,
            .parameters = parameters_,
        };
    }

    bool set_parameter(const char* name, double value) noexcept override {
        bool success = false;
        for_each(parameter_values_, [&, index = 0]<typename T>(T& parameter_value) mutable {
            if (equal(name, parameters_[index].name)) {
                parameter_value = static_cast<T>(value);
                success = true;
            }
            ++index;
        });
        return success;
    }
    
    std::optional<double> get_parameter(const char* name) const noexcept override {
        std::optional<double> value;
        for_each(parameter_values_, [&, index = 0](const auto& parameter_value) mutable {
            if (equal(name, parameters_[index].name)) {
                value = static_cast<double>(parameter_value);
            }
            ++index;
        });
        return value;
    }

    double process(Env& env, Input input) override {
        return std::apply(
            [&](const auto&... args) { return feature_->func(env, input, args...); },
            parameter_values_
        );
    }

private:
    const Feature<R, Args...>* feature_;
    std::array<ParameterDescriptor, sizeof...(Args)> parameters_;
    std::tuple<std::remove_cvref_t<Args>...> parameter_values_;
};

std::unique_ptr<Algorithm> make_algorithm(const char* identifier) {
    static constexpr auto feature_meta = meta();

    std::unique_ptr<Algorithm> result;
    for_each(feature_meta, [&](const auto& feature) {
        if (std::string_view{feature.identifier} == std::string_view{identifier}) {
            result = std::unique_ptr<Algorithm>(new AlgorithmAdapter(feature));
        }
    });
    return result;
}

}  // namespace openae::features
