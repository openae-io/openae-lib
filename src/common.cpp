#include "openae/common.hpp"

#include <memory>
#include <source_location>

#include "cache.hpp"

namespace openae {

template <typename T>
static void delete_func(T* ptr) {
    delete ptr;  // NOLINT(cppcoreguidelines-owning-memory)
}

std::unique_ptr<Cache, void (*)(Cache*)> make_cache() {
    return {new Cache, &delete_func<Cache>};
}

void log(Env& env, LogLevel level, const char* msg) {
    if (env.logger != nullptr) {
        env.logger(level, msg);
    }
}

}  // namespace openae
