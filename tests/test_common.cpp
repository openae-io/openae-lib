#include <catch2/catch_test_macros.hpp>

#include "openae/common.hpp"

#include "cache.hpp"

static int square(int x) {
    return x + x;
}

static int increment(int x) {
    return x + 1;
}

TEST_CASE("RingBufferStorage") {
    openae::RingBufferStorage<int, float, 3> storage;
    CHECK(storage.size() == 0);

    SECTION("insert with same key") {
        CHECK(storage.insert(1, 1.1f) == 1.1f);
        CHECK(storage.size() == 1);

        CHECK(storage.insert(1, 2.2f) == 2.2f);
        CHECK(storage.size() == 1);
    }

    SECTION("insert with different keys") {
        CHECK(storage.insert(1, 1.1f) == 1.1f);
        CHECK(storage.size() == 1);

        CHECK(storage.insert(2, 2.2f) == 2.2f);
        CHECK(storage.size() == 2);
    }

    SECTION("insert with overflow") {
        storage.insert(1, {});
        CHECK(storage.size() == 1);
        storage.insert(2, {});
        CHECK(storage.size() == 2);
        storage.insert(3, {});
        CHECK(storage.size() == 3);
        storage.insert(4, {});
        CHECK(storage.size() == 3);

        CHECK(storage.find(1) == nullptr);
        CHECK(storage.find(2) != nullptr);
        CHECK(storage.find(3) != nullptr);
        CHECK(storage.find(4) != nullptr);
    }

    SECTION("find") {
        CHECK(storage.find(1) == nullptr);

        CHECK(storage.insert(1, 1.1f) == 1.1f);
        CHECK(storage.find(1) != nullptr);
        CHECK(*storage.find(1) == 1.1f);
    }
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
