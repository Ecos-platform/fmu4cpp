set(generatedSourcesDir "${CMAKE_BINARY_DIR}/generated")

set(lib_info "${generatedSourcesDir}/fmu4cpp/lib_info.cpp")
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/fmu4cpp/lib_info.cpp.in"
        "${lib_info}"
        @ONLY
)

set(publicHeaders
        "fmi2/fmi2Functions.h"
        "fmi2/fmi2FunctionTypes.h"
        "fmi2/fmi2TypesPlatform.h"
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
        )

set(sources
        "fmu4cpp/fmi2.cpp"
        "fmu4cpp/fmu_base.cpp"
        "fmu4cpp/fmu_variable.cpp"
        )

set(publicHeadersFull)
foreach(h IN LISTS publicHeaders)
    list(APPEND publicHeadersFull "${CMAKE_CURRENT_SOURCE_DIR}/../include/${h}")
endforeach()

set(privateHeadersFull)
foreach(h IN LISTS privateHeaders)
    list(APPEND privateHeadersFull "${CMAKE_CURRENT_SOURCE_DIR}/${h}")
endforeach()

add_library(fmu4cpp OBJECT
        "${lib_info}"
        "${publicHeadersFull}"
        "${privateHeadersFull}"
        "${sources}"
        )
target_compile_features(fmu4cpp PUBLIC "cxx_std_17")
set_target_properties(fmu4cpp PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(fmu4cpp
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )
