cmake_minimum_required(VERSION 3.20) # should be 3.23 but Python build fails with scikit-build-core

project(
    openae
    VERSION 0.1.0
    DESCRIPTION "OpenAE implementation"
    HOMEPAGE_URL "https://openae.io"
    LANGUAGES CXX
)

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

# dependencies
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY      https://github.com/fmtlib/fmt
  GIT_TAG             11.1.4
  EXCLUDE_FROM_ALL
  SYSTEM
  FIND_PACKAGE_ARGS   11.0.0...12.0.0
)
FetchContent_MakeAvailable(fmt)

option(XXHASH_BUILD_XXHSUM "Build the xxhsum binary" OFF)
FetchContent_Declare(
    xxHash
    GIT_REPOSITORY    https://github.com/Cyan4973/xxHash.git
    GIT_TAG           v0.8.3
    SOURCE_SUBDIR     cmake_unofficial
    EXCLUDE_FROM_ALL
    SYSTEM
    FIND_PACKAGE_ARGS 0.8.3...0.9.0
)
FetchContent_MakeAvailable(xxHash)

# library
add_subdirectory(src)

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
