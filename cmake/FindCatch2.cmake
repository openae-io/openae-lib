find_package(Catch2 CONFIG QUIET)

if (NOT Catch2_FOUND)
    message(STATUS "Fetch Catch2")

    include(FetchContent)
    option(CATCH_INSTALL_DOCS "Install documentation alongside library" OFF)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.8.0
    )
    FetchContent_MakeAvailable(Catch2)
endif()
