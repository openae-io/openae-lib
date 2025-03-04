find_package(Catch2 CONFIG QUIET)

if (NOT Catch2_FOUND)
    message(STATUS "Fetch Catch2")

    include(FetchContent)
    option(CATCH_INSTALL_DOCS "Install documentation alongside library" OFF)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.8.0
        EXCLUDE_FROM_ALL
        SYSTEM
    )
    FetchContent_MakeAvailable(Catch2)
    set_target_properties(Catch2 PROPERTIES CXX_CLANG_TIDY "")
    set_target_properties(Catch2WithMain PROPERTIES CXX_CLANG_TIDY "")
endif()
