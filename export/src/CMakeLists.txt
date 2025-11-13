
set(lib_info "${generatedSourcesDir}/fmu4cpp/lib_info.cpp")
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/fmu4cpp/lib_info.cpp.in"
        "${lib_info}"
        @ONLY
)

set(fmi2Headers
        "fmi2/fmi2Functions.h"
        "fmi2/fmi2FunctionTypes.h"
        "fmi2/fmi2TypesPlatform.h"
)

set(fmi3Headers
        "fmi3/fmi3Functions.h"
        "fmi3/fmi3FunctionTypes.h"
        "fmi3/fmi3PlatformTypes.h"
)

set(publicHeaders
        "fmu4cpp/lib_info.hpp"
        "fmu4cpp/logger.hpp"
        "fmu4cpp/model_info.hpp"
        "fmu4cpp/fmu_base.hpp"
        "fmu4cpp/fmu_except.hpp"
        "fmu4cpp/fmu_variable.hpp"
)

set(privateHeaders
        "fmu4cpp/hash.hpp"
        "fmu4cpp/time.hpp"
        "fmu4cpp/util.hpp"
)

set(sources
        "fmu4cpp/fmu_base.cpp"
        "fmu4cpp/fmu_variable.cpp"
)

set(fmi2HeadersFull)
foreach (h IN LISTS fmi2Headers)
    list(APPEND fmi2HeadersFull "${CMAKE_CURRENT_SOURCE_DIR}/../include/${h}")
endforeach ()

set(fmi3HeadersFull)
foreach (h IN LISTS fmi3Headers)
    list(APPEND fmi3HeadersFull "${CMAKE_CURRENT_SOURCE_DIR}/../include/${h}")
endforeach ()

set(publicHeadersFull)
foreach (h IN LISTS publicHeaders)
    list(APPEND publicHeadersFull "${CMAKE_CURRENT_SOURCE_DIR}/../include/${h}")
endforeach ()

set(privateHeadersFull)
foreach (h IN LISTS privateHeaders)
    list(APPEND privateHeadersFull "${CMAKE_CURRENT_SOURCE_DIR}/${h}")
endforeach ()

set(sourcesFull)
foreach (h IN LISTS sources)
    list(APPEND sourcesFull "${CMAKE_CURRENT_SOURCE_DIR}/${h}")
endforeach ()

add_library(fmu4cpp_base OBJECT
        "${lib_info}"
        "${publicHeadersFull}"
        "${privateHeadersFull}"
        "${sourcesFull}"
)
target_compile_features(fmu4cpp_base PRIVATE "cxx_std_17")
set_target_properties(fmu4cpp_base PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(fmu4cpp_base
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_library(fmu4cpp_fmi2 OBJECT
        "${fmi2HeadersFull}"
        "fmu4cpp/fmi2/fmi2.cpp"
        "fmu4cpp/fmi2/fmi2_description.cpp"
)
target_compile_features(fmu4cpp_fmi2 PRIVATE "cxx_std_17")
set_target_properties(fmu4cpp_fmi2 PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(fmu4cpp_fmi2
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/fmi2>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_library(fmu4cpp_fmi3 OBJECT
        "${fmi3HeadersFull}"
        "fmu4cpp/fmi3/fmi3.cpp"
        "fmu4cpp/fmi3/fmi3_description.cpp"
)
target_compile_features(fmu4cpp_fmi3 PRIVATE "cxx_std_17")
set_target_properties(fmu4cpp_fmi3 PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(fmu4cpp_fmi3
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/fmi3>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)
