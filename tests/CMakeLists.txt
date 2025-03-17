include(FetchContent)

option(CATCH_INSTALL_DOCS "Install documentation alongside library" OFF)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
    GIT_TAG           v3.8.0
    EXCLUDE_FROM_ALL
    SYSTEM
    FIND_PACKAGE_ARGS 3.8.0...4.0.0
)
FetchContent_MakeAvailable(Catch2)
if(TARGET Catch2)
    set_target_properties(Catch2 Catch2WithMain PROPERTIES CXX_CLANG_TIDY "")
endif()

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY    https://github.com/marzer/tomlplusplus.git
    GIT_TAG           v3.4.0
    EXCLUDE_FROM_ALL
    SYSTEM
    FIND_PACKAGE_ARGS 3.4.0...4.0.0
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
