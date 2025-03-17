find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)
find_package(nanobind REQUIRED)

nanobind_add_module(
    openae_bindings_python
    STABLE_ABI
    NB_STATIC
    NB_SUPPRESS_WARNINGS
    LTO
)
target_sources(
    openae_bindings_python
    PRIVATE
        src/openae.cpp
)
target_link_libraries(
    openae_bindings_python
    PRIVATE
        openae_project_options
        openae
        fmt::fmt
)
set_target_properties(
    openae_bindings_python
    PROPERTIES
        OUTPUT_NAME openae
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

nanobind_add_stub(
    openae_bindings_python_stub_common
    MODULE openae
    OUTPUT stub/openae/__init__.pyi
    MARKER_FILE stub/openae/py.typed
    PYTHON_PATH $<TARGET_FILE_DIR:openae_bindings_python>
    DEPENDS openae_bindings_python
)

nanobind_add_stub(
    openae_bindings_python_stub_features
    MODULE openae.features
    OUTPUT stub/openae/features.pyi
    PYTHON_PATH $<TARGET_FILE_DIR:openae_bindings_python>
    DEPENDS openae_bindings_python
)

if(SKBUILD)
    install(
        TARGETS openae_bindings_python
        LIBRARY DESTINATION .
        COMPONENT bindings
    )
    install(
        DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/stub/"
        DESTINATION .
        COMPONENT bindings
    )
endif()
