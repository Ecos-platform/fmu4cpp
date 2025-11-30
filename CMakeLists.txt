cmake_minimum_required(VERSION 3.15)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" projectVersion)
project(fmu4cpp-template VERSION ${projectVersion})

option(FMU4CPP_BUILD_TESTS "Build internal tests" OFF)
option(FMU4CPP_MSVC_STATIC_CRT "Use static MSVC runtime library" ON)

set(CMAKE_CXX_STANDARD 17)

if (POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
    if (WIN32)
        if (FMU4CPP_MSVC_STATIC_CRT)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else ()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif ()
    endif ()
endif ()


include("cmake/generate_fmu.cmake")

add_subdirectory(export)
add_subdirectory(examples)
