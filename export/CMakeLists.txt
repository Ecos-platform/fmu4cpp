set(generatedSourcesDir "${CMAKE_BINARY_DIR}/generated")

add_subdirectory(src)

# internal executable invoked by CMake to generate the modelDescription.xml from a shared library
add_executable(descriptionGenerator "descriptionGenerator.cpp")
if (UNIX)
    target_link_libraries(descriptionGenerator PRIVATE dl)
endif ()

if (FMU4CPP_BUILD_TESTS)
    Include(FetchContent)

    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)

    enable_testing()
    add_subdirectory(tests)
endif ()
