add_executable(openae_vamp_test_plugin test_plugin.cpp ../src/plugin.cpp)
target_link_libraries(
    openae_vamp_test_plugin
    PRIVATE
        openae_project_options
        openae::openae
        Catch2::Catch2WithMain
)
target_include_directories(openae_vamp_test_plugin PRIVATE ../src)

include(Catch)
catch_discover_tests(openae_vamp_test_plugin)
