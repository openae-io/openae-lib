#pragma once

#include <functional>
#include <memory>
#include <memory_resource>
#include <source_location>

#include "openae/config.hpp"

namespace openae {

enum class LogLevel {
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
};

using Logger =
    std::function<void(LogLevel level, const char* message, std::source_location location)>;

using MemoryResource = std::pmr::memory_resource;

struct Cache;

OPENAE_EXPORT std::unique_ptr<Cache, void (*)(Cache*)> make_cache();

struct Env {
    Logger logger = nullptr;
    MemoryResource* mem_resource = nullptr;
    Cache* cache = nullptr;
};

OPENAE_EXPORT void log(
    Env& env,
    LogLevel level,
    const char* msg,
    std::source_location location = std::source_location::current()
);

}  // namespace openae
