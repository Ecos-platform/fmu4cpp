cmake_minimum_required(VERSION 3.15)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" projectVersion)
project(fmu4cpp-template VERSION ${projectVersion})

option(FMU4CPP_BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(modelIdentifier identity)
set(MODEL_INFO "${CMAKE_CURRENT_SOURCE_DIR}/model_info.json")

if (MSVC)
    # https://stackoverflow.com/questions/14172856/compile-with-mt-instead-of-md-using-cmake
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            )
    foreach (CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach ()
endif ()

if (WIN32)
    set(TARGET_PLATFORM win)
elseif (APPLE)
    set(TARGET_PLATFORM darwin)
else ()
    set(TARGET_PLATFORM linux)
endif ()

if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(TARGET_PLATFORM ${TARGET_PLATFORM}64)
else ()
    set(TARGET_PLATFORM ${TARGET_PLATFORM}32)
endif ()

add_subdirectory(export)
add_subdirectory(src)
