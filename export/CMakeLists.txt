set(generatedSourcesDir "${CMAKE_BINARY_DIR}/generated")

set(lib_info "${generatedSourcesDir}/fmu4cpp/lib_info.cpp")
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/fmu4cpp/lib_info.cpp.in"
        "${lib_info}"
        @ONLY
)

set(model_info "${generatedSourcesDir}/fmu4cpp/model_info.cpp")
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/fmu4cpp/model_info.cpp.in"
        "${model_info}"
        @ONLY
)

set(headers
        "fmu4cpp/lib_info.hpp"
        "fmu4cpp/model_info.hpp"
        "fmu4cpp/fmu_base.hpp"
        "fmu4cpp/fmu_except.hpp"
        "fmu4cpp/fmu_variable.hpp"
        "fmu4cpp/time.hpp"
        )

set(sources
        "fmu4cpp/fmi2.cpp"
        "fmu4cpp/fmu_base.cpp"
        "fmu4cpp/fmu_variable.cpp"
        )

add_library(fmu4cpp OBJECT
        "${lib_info}"
        "${model_info}"
        "${headers}"
        "${sources}"
        )
target_compile_features(fmu4cpp PUBLIC "cxx_std_17")
set_target_properties(fmu4cpp PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(fmu4cpp PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(fmu4cpp
        PRIVATE
        "${PROJECT_SOURCE_DIR}/fmi_headers"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )

add_executable(descriptionGenerator descriptionGenerator.cpp)
if (UNIX)
    target_link_libraries(descriptionGenerator PRIVATE dl)
endif()
