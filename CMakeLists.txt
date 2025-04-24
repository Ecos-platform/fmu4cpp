cmake_minimum_required(VERSION 3.15)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" projectVersion)
project(fmu4cpp-template VERSION ${projectVersion})

if (POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
    if (WIN32)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif ()
endif ()

option(FMU4CPP_BUILD_TESTS "Build internal tests" OFF)

set(CMAKE_CXX_STANDARD 17)


############modelIdentifier and export version###########

set(modelIdentifier identity) # <-- CHANGE ME
set(fmi_version "fmi2") # fmi2 and fmi3 is supported

########################################################

include("cmake/generate_fmu.cmake")

add_subdirectory(export)
add_subdirectory(src)
