cmake_minimum_required(VERSION 3.15)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" projectVersion)
project(fmu4cpp-template VERSION ${projectVersion})

option(FMU4CPP_BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(modelIdentifier identity) # <-- CHANGE ME

include("cmake/generate_fmu.cmake")

if (MSVC)
    # link statically against the the Visual C runtime
    string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    string(REPLACE "/MDd" "/MTd" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
endif ()

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
