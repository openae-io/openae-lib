find_package(tomlplusplus CONFIG QUIET)

if (NOT tomlplusplus_FOUND)
    message(STATUS "Fetch tomlplusplus")

    include(FetchContent)
    FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG        v3.4.0
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(tomlplusplus)
endif()
