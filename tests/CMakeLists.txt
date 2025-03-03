find_package(Catch2 REQUIRED)
find_package(tomlplusplus REQUIRED)

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
