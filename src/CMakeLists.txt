include(FetchContent)
option(XXHASH_BUILD_XXHSUM "Build the xxhsum binary" OFF)
FetchContent_Declare(
    xxHash
    GIT_REPOSITORY    https://github.com/Cyan4973/xxHash.git
    GIT_TAG           v0.8.3
    SOURCE_SUBDIR     cmake_unofficial
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS 0.8.3
)
FetchContent_MakeAvailable(xxHash)

configure_file(
    "${PROJECT_SOURCE_DIR}/include/openae/config.hpp.in"
    "${PROJECT_BINARY_DIR}/include/openae/config.hpp"
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
                "${PROJECT_BINARY_DIR}/include"
                "${PROJECT_SOURCE_DIR}/include"
            FILES
                "${PROJECT_BINARY_DIR}/include/openae/config.hpp"
                "${PROJECT_SOURCE_DIR}/include/openae/common.hpp"
                "${PROJECT_SOURCE_DIR}/include/openae/features.hpp"
    PRIVATE
        common.cpp
        features.cpp
)
target_link_libraries(
    openae
    PRIVATE
        $<BUILD_INTERFACE:openae_project_options>
    PUBLIC
        # TODO: xxhash.h not found if PRIVATE
        $<BUILD_INTERFACE:xxHash::xxhash>  # effectively header-only with XXH_INLINE_ALL
)
set_target_properties(
    openae
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        POSITION_INDEPENDENT_CODE ON
        VERIFY_INTERFACE_HEADER_SETS ON
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
install(
    TARGETS openae
    EXPORT openaeTargets
    FILE_SET openae_headers
)
install(
    EXPORT openaeTargets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/openae"
    NAMESPACE openae::
)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/openaeConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/cmake/openaeConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/openae"
    PATH_VARS PROJECT_NAME PROJECT_VERSION
)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/cmake/openaeConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)
install(
    FILES
        "${PROJECT_BINARY_DIR}/cmake/openaeConfig.cmake"
        "${PROJECT_BINARY_DIR}/cmake/openaeConfigVersion.cmake"
    DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/openae"
)
