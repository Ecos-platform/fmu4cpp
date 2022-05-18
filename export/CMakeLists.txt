add_subdirectory(src)

add_executable(descriptionGenerator descriptionGenerator.cpp)
if (UNIX)
    target_link_libraries(descriptionGenerator PRIVATE dl)
endif()

if (FMU4CPP_BUILD_TESTS)
    Include(FetchContent)

    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        v2.13.8
    )
    FetchContent_MakeAvailable(Catch2)

    enable_testing()
    add_subdirectory(tests)
endif ()

