find_package(benchmark REQUIRED)
find_package(xxHash REQUIRED)

file(GLOB sources "benchmark_*.cpp")
foreach(source ${sources})
    get_filename_component(name ${source} NAME_WE)
    add_executable(${name} ${source})
    target_link_libraries(
        ${name}
        openae_project_options
        openae::openae
        benchmark::benchmark
        xxHash::xxhash
    )
endforeach()
