#include <catch2/catch_test_macros.hpp>

#include "openae/common.hpp"

#include "cache.hpp"

static int square(int x) {
    return x + x;
}

static int increment(int x) {
    return x + 1;
}

TEST_CASE("Cache") {
    auto cache = openae::make_cache();
    CHECK(square(2) == 4);
    CHECK(openae::cached(cache.get(), square, 2) == 4);
    CHECK(openae::cached(cache.get(), square, 2) == 4);

    SECTION("Distinct caches for different function with same return type") {
        CHECK(openae::cached(cache.get(), square, 2) == 4);
        CHECK(openae::cached(cache.get(), increment, 2) == 3);
    }
}
