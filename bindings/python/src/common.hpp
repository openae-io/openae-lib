#include <memory_resource>

#include <nanobind/nanobind.h>
#include <openae/common.hpp>

namespace nb = nanobind;

constexpr int py_log_level(openae::LogLevel level) noexcept {
    // https://docs.python.org/3/library/logging.html#logging-levels
    switch (level) {
    case openae::LogLevel::Trace:
    case openae::LogLevel::Debug:
        return 10;
    case openae::LogLevel::Info:
        return 20;
    case openae::LogLevel::Warning:
        return 30;
    case openae::LogLevel::Error:
        return 40;
    case openae::LogLevel::Fatal:
        return 50;
    default:
        return 0;
    }
}

inline void py_log(
    openae::LogLevel level, const char* msg, [[maybe_unused]] std::source_location location
) {
    auto logger = nb::module_::import_("logging").attr("getLogger")("openae");
    // https://docs.python.org/3/library/logging.html#logrecord-objects
    nb::dict kwargs;  // NOLINT(*const-correctness), false positive
    kwargs["extra"] = nb::dict{};
    logger.attr("log")(py_log_level(level), msg, **kwargs);
}

inline openae::Env& py_env() {
    static openae::Env env{
        .logger = py_log,
        .mem_resource = std::pmr::new_delete_resource(),
        .cache = nullptr,
    };
    return env;
}
