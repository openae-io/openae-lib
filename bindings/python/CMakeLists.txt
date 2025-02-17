find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)
find_package(nanobind REQUIRED)

nanobind_add_module(
    openae_bindings_python
    STABLE_ABI
    NB_STATIC
    NB_SUPPRESS_WARNINGS
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
set_target_properties(openae_bindings_python PROPERTIES OUTPUT_NAME "_openae")

if(SKBUILD)
    install(TARGETS openae_bindings_python LIBRARY DESTINATION openae)
endif()
