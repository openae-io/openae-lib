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
        openae
        openae_project_options
)
set_target_properties(
    openae_bindings_python
    PROPERTIES
        OUTPUT_NAME openae
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

if(OPENAE_BUILD_PYTHON_STUBS)
    nanobind_add_stub(
        openae_bindings_python_stub_common
        MODULE openae
        OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/openae/__init__.pyi"
        PYTHON_PATH $<TARGET_FILE_DIR:openae_bindings_python>
        DEPENDS openae_bindings_python
    )

    nanobind_add_stub(
        openae_bindings_python_stub_features
        MODULE openae.features
        OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/openae/features.pyi"
        PYTHON_PATH $<TARGET_FILE_DIR:openae_bindings_python>
        DEPENDS openae_bindings_python
    )
endif()

if(SKBUILD)
    install(
        TARGETS openae_bindings_python
        LIBRARY DESTINATION .
        COMPONENT bindings
    )
    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/openae"
        DESTINATION .
        COMPONENT bindings
    )
endif()
