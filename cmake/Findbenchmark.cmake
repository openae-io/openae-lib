find_package(benchmark CONFIG QUIET)

if (NOT benchmark_FOUND)
    message(STATUS "Fetch benchmark")

    include(FetchContent)
    option(BENCHMARK_ENABLE_TESTING "Enable testing of the benchmark library." OFF)
    option(BENCHMARK_INSTALL_DOCS "Enable installation of documentation." OFF)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.9.1
        EXCLUDE_FROM_ALL
        SYSTEM
    )
    FetchContent_MakeAvailable(benchmark)
endif()
