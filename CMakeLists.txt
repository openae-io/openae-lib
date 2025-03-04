cmake_minimum_required(VERSION 3.20) # should be 3.23 but Python build fails with scikit-build-core

project(
    openae
    VERSION 0.1.0
    DESCRIPTION "OpenAE implementation"
    HOMEPAGE_URL "https://openae.io"
    LANGUAGES CXX
)

message(STATUS "Host: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "Target: ${CMAKE_SYSTEM_PROCESSOR}")

# set c++ standard explicitly, compile feature "cxx_std_20" does not set -std=c++20 compile flag
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(OPENAE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

option(OPENAE_ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
if(OPENAE_ENABLE_CLANG_TIDY)
    message(STATUS "Static code analysis with clang-tidy enabled")
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
        if(OPENAE_WARNINGS_AS_ERRORS)
            set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-warnings-as-errors=*")
        else()
            set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
        endif()
    else()
        message(SEND_ERROR "clang-tidy requested but executable not found")
    endif()
endif()

# interface "library" to set the c++ standard / compile-time options requested
add_library(openae_project_options INTERFACE)
target_compile_features(openae_project_options INTERFACE cxx_std_20)

if(MSVC)
    set(warnings
        /permissive-
        /W4
    )
    if(OPENAE_WARNINGS_AS_ERRORS)
        list(APPEND warnings /WX)
    endif()
    target_compile_options(openae_project_options INTERFACE ${warnings})
else()
    set(warnings
        -Wall
        -Wextra
        -Wshadow
        -Wnon-virtual-dtor
        -Wpedantic
    )
    if(OPENAE_WARNINGS_AS_ERRORS)
        list(APPEND warnings -Werror)
    endif()
    target_compile_options(openae_project_options INTERFACE ${warnings})
endif()

find_package(xxHash REQUIRED)

configure_file(
    ${PROJECT_SOURCE_DIR}/include/openae/config.hpp.in
    ${PROJECT_BINARY_DIR}/include/openae/config.hpp
    @ONLY
)

add_library(openae)
add_library(openae::openae ALIAS openae)
target_sources(
    openae
    PUBLIC
        FILE_SET openae_headers
            TYPE HEADERS
            BASE_DIRS
                ${PROJECT_BINARY_DIR}/include
                ${PROJECT_SOURCE_DIR}/include
            FILES
                ${PROJECT_BINARY_DIR}/include/openae/config.hpp
                ${PROJECT_SOURCE_DIR}/include/openae/common.hpp
                ${PROJECT_SOURCE_DIR}/include/openae/features.hpp
    PRIVATE
        src/common.cpp
        src/features.cpp
)
target_link_libraries(
    openae
    PRIVATE
        $<BUILD_INTERFACE:openae_project_options>
    PUBLIC
        xxHash::xxhash  # TODO: header xxhash.h not found if PRIVATE
)
set_target_properties(
    openae
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        POSITION_INDEPENDENT_CODE ON
        VERIFY_INTERFACE_HEADER_SETS ON
)

# tests
option(OPENAE_BUILD_TESTS "Build unit tests" OFF)
if(OPENAE_BUILD_TESTS)
    message(STATUS "Unit tests enabled")
    enable_testing()
    add_subdirectory(tests)
endif()

# benchmarks
option(OPENAE_BUILD_BENCHMARKS "Build benchmarks" OFF)
if(OPENAE_BUILD_BENCHMARKS)
    message(STATUS "Benchmarks enabled")
    add_subdirectory(benchmarks)
endif()

# bindings
option(OPENAE_BUILD_PYTHON "Build Python bindings" OFF)
if(OPENAE_BUILD_PYTHON)
    add_subdirectory(bindings/python)
endif()
