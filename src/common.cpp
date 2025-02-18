#include "openae/common.hpp"

#include "cache.hpp"

namespace openae {

template <typename T>
static void delete_func(T* ptr) {
    delete ptr;
}

std::unique_ptr<Cache, void(*)(Cache*)> make_cache() {
    return {new Cache, &delete_func<Cache>};
}

void log(Env& env, LogLevel level, const char* msg, std::source_location location) {
    if (env.logger != nullptr) {
        env.logger(level, msg, location);
    }
}

}  // namespace openae
