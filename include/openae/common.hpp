#pragma once

#include <functional>
#include <memory>
#include <memory_resource>

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

using Logger = std::function<void(LogLevel, const char*)>;

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
