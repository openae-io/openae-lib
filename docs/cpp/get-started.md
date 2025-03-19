# Get Started

The library can be built, integrated and installed using [CMake](https://cmake.org/runningcmake/) and a C++20 compiler.
Following build options are available:

- `OPENAE_BUILD_TESTS`: Build unit tests
- `OPENAE_BUILD_BENCHMARKS`: Build benchmarks
- `OPENAE_BUILD_PYTHON`: Build Python bindings
- `OPENAE_WARNINGS_AS_ERRORS`: Treat warnings as errors
- `OPENAE_ENABLE_CLANG_TIDY`: Enable static analysis with Clang-Tidy

Requirements:
- CMake 3.24 or higher
- C++20 compiler

## Integrate as a pre-compiled library

If you build and install this package to your system, a `openaeConfig.cmake` file will be generated and installed to your system.
The installed library can be found and linked within CMake:

```cmake
find_package(openae CONFIG REQUIRED)
target_link_libraries(myexecutable PRIVATE openae::openae)
```

## Integrate as an embedded (in-source) dependency

Add it to your project as a Git submodule (`git submodule add https://github.com/openae-io/openae-lib.git`) and link it with CMake:

```cmake
add_subdirectory(extern/openae-lib)  # the submodule directory
target_link_libraries(myexecutable PRIVATE openae::openae)
```

## Build and install

```shell
# clone repository
git clone --recursive https://github.com/openae-io/openae-lib.git
cd openae-lib

# build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DOPENAE_BUILD_TESTS=ON ..
cmake --build .                   # single-configuration generator like Make or Ninja
cmake --build . --config Release  # multi-configuration generator like Visual Studio, Xcode

# run tests
ctest --output-on-failure

# install
cmake --install . --config Release
```
