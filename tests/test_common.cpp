#include <catch2/catch_test_macros.hpp>

#include "openae/common.hpp"

#include "cache.hpp"

static int square(int x) {
    return x * x;
}

TEST_CASE("Cache") {
    auto cache = openae::make_cache();
    CHECK(square(2) == 4);
    CHECK(openae::cached(cache.get(), square, 2) == 4);
    CHECK(openae::cached(cache.get(), square, 2) == 4);
}
