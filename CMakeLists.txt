cmake_minimum_required(VERSION 3.15)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" projectVersion)
project(fmu4cpp-template VERSION ${projectVersion})

option(FMU4CPP_BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(modelIdentifier identity)

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

if (FMU4CPP_BUILD_TESTS)
    Include(FetchContent)

    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v2.13.8
    )
    FetchContent_MakeAvailable(Catch2)

    enable_testing()
    add_subdirectory(export/tests)
endif ()
