cmake_minimum_required(VERSION 3.15)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" projectVersion)
project(fmu4cpp-template VERSION ${projectVersion})


option(FMU4CPP_BUILD_EXAMPLES "Build example FMUs" OFF)
option(FMU4CPP_BUILD_TESTS "Build internal tests" OFF)

set(CMAKE_CXX_STANDARD 17)

if (POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
    if (WIN32)
        # Single-config generators (Ninja, Make): pick based on CMAKE_BUILD_TYPE
        if (NOT CMAKE_CONFIGURATION_TYPES)
            if (CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL" CACHE STRING "" FORCE)
            else()
                set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)
            endif()
        endif()
    endif ()
endif ()


set(generatedSourcesDir "${CMAKE_BINARY_DIR}/generated")
set(fmuResultDir "${CMAKE_BINARY_DIR}/models")

file(MAKE_DIRECTORY ${generatedSourcesDir})
file(MAKE_DIRECTORY ${fmuResultDir})

include("cmake/generate_fmu.cmake")

add_subdirectory(export)

if (FMU4CPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
