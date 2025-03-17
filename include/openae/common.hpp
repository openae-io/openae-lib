#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <memory_resource>

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

using Logger = std::function<void(LogLevel level, const char* message)>;

using MemoryResource = std::pmr::memory_resource;

struct Cache;

OPENAE_EXPORT std::unique_ptr<Cache, void (*)(Cache*)> make_cache();

struct Env {
    Logger logger = nullptr;
    MemoryResource* mem_resource = nullptr;
    Cache* cache = nullptr;
};

OPENAE_EXPORT void log(Env& env, LogLevel level, const char* msg);

}  // namespace openae
