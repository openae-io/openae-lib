# Vamp plugin module exposing OpenAE features.
#
# Produces a single shared module that exports vampGetPluginDescriptor.
# The file name defines the plugin library id, so it must be exactly:
#   Linux   : openae.so
#   macOS   : openae.dylib
#   Windows : openae.dll
# To avoid clashes with the core openae library, the module gets its own output directory in the build tree and its own install destination.

if(BUILD_SHARED_LIBS)
    message(WARNING "Vamp plugin depends on shared openae library and will not be loadable by hosts; use BUILD_SHARED_LIBS=OFF")
endif()

add_library(openae_vamp MODULE src/plugin.cpp src/vamp.h)
target_link_libraries(openae_vamp PRIVATE openae::openae openae_project_options)
target_include_directories(openae_vamp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(
    openae_vamp
    PROPERTIES
        PREFIX ""
        OUTPUT_NAME "openae"
        CXX_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/vamp"
        ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/vamp"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/vamp"
)
if(APPLE)
    # MODULE targets default to .so on macOS, but Vamp hosts scan for .dylib.
    set_target_properties(openae_vamp PROPERTIES SUFFIX ".dylib")
endif()

include(GNUInstallDirs)
install(
    TARGETS openae_vamp
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/vamp" COMPONENT vamp
)

if(OPENAE_BUILD_TESTS)
    add_subdirectory(tests)
endif()
