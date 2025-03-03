find_package(xxHash CONFIG QUIET)

if (NOT xxHash_FOUND)
    message(STATUS "Fetch xxHash")

    include(FetchContent)
    option(XXHASH_BUILD_XXHSUM "Build the xxhsum binary" OFF)
    FetchContent_Declare(
        xxHash
        GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
        GIT_TAG        v0.8.3
        SOURCE_SUBDIR  cmake_unofficial
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(xxHash)
endif()
