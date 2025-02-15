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
if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()


############modelIdentifier###################

set(modelIdentifier identity) # <-- CHANGE ME

################################################

include("cmake/generate_fmu.cmake")

if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(BITNESS 64)
else ()
    set(BITNESS 32)
endif ()

if (WIN32)
    set(TARGET_PLATFORM win${BITNESS})
elseif (APPLE)
    set(TARGET_PLATFORM darwin${BITNESS})
else ()
    set(TARGET_PLATFORM linux${BITNESS})
endif ()

add_subdirectory(export)
add_subdirectory(src)
