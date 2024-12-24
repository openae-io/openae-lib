find_package(Catch2 REQUIRED)

# toml++ vcpkg port currently relies on PkgConfig, use FetchContent instead
# https://github.com/marzer/tomlplusplus/issues/209
include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

configure_file(test_config.hpp.in test_config.hpp @ONLY)

add_executable(openae_test_common test_common.cpp)
target_link_libraries(
    openae_test_common
    PRIVATE
        openae_project_options
        openae::openae
        Catch2::Catch2WithMain
)
target_include_directories(openae_test_common PRIVATE ../src)

add_executable(openae_test_features test_features.cpp)
target_link_libraries(
    openae_test_features
    PRIVATE
        openae_project_options
        openae::openae
        Catch2::Catch2WithMain
        tomlplusplus::tomlplusplus
)
target_include_directories(openae_test_features PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

include(CTest)
include(Catch)
catch_discover_tests(openae_test_common)
catch_discover_tests(openae_test_features)
