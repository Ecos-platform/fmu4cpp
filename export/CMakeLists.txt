add_subdirectory(src)
add_subdirectory(examples)

# internal executable invoked by CMake to generate the modelDescription.xml from a shared library
add_executable(descriptionGenerator "descriptionGenerator.cpp")
if (UNIX)
    target_link_libraries(descriptionGenerator PRIVATE dl)
endif()

if (FMU4CPP_BUILD_TESTS)
    Include(FetchContent)

    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)

    FetchContent_Declare(
            fmi4c
            GIT_REPOSITORY https://github.com/robbr48/fmi4c.git
            GIT_TAG 6f7f449ff03a78b0533a8483cafe1fd9965f4e6e
    )
    FetchContent_MakeAvailable(fmi4c)

    enable_testing()
    add_subdirectory(tests)
endif ()
