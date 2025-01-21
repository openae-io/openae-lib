#pragma once

#include <functional>
#include <memory>
#include <memory_resource>

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

std::unique_ptr<Cache, void(*)(Cache*)> make_cache();

struct Env {
    LogLevel log_level = LogLevel::Info;
    Logger logger = nullptr;
    MemoryResource* mem_resource = nullptr;
    Cache* cache = nullptr;
};

void log(Env& env, LogLevel level, const char* msg);

}  // namespace openae
