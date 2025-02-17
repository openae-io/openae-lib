# OpenAE Library

The OpenAE library is a high-performance implementation of the [OpenAE](https://openae.io) standards for real-time acoustic emission signal processing:

- [Feature extraction algorithms](https://openae.io/features/latest/)

The library is implemented in C++20 and offers bindings for:

- [x] Python
- [ ] MATLAB
- [ ] C
- [ ] Web Assembly
- [ ] Node.js

## 🚀 Getting started

The library can be built, integrated and installed using [CMake](https://cmake.org/runningcmake/) and a C++20 compiler.
Following build options are available:

- `OPENAE_BUILD_PYTHON`: Build Python bindings
- `OPENAE_BUILD_BENCHMARKS`: Build benchmarks
- `OPENAE_BUILD_TESTS`: Build unit tests

### Integrate as an embedded (in-source) dependency

Add it to your project as a Git submodule (`git submodule add https://github.com/openae-io/openae-lib.git`) and link it with CMake:

```cmake
add_subdirectory(extern/openae-lib)  # the submodule directory
target_link_libraries(myexecutable PRIVATE openae::openae)
```

### Build and install

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
```
