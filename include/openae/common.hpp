#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <memory_resource>
#include <source_location>

#include "openae/config.hpp"

namespace openae {

enum class LogLevel : std::uint8_t {
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
};

/// Log function.
using Logger =
std::function<void(LogLevel level, const char* message, std::source_location location)>;

/// Memory resource.
using MemoryResource = std::pmr::memory_resource;

/// Cache (opaque type).
struct Cache;

/// Create cache.
OPENAE_EXPORT std::unique_ptr<Cache, void (*)(Cache*)> make_cache();

/// The Env structure serves as a (shared) execution context.
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
