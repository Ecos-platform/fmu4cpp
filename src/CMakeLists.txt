include("${PROJECT_SOURCE_DIR}/cmake/generate_fmu.cmake")

add_library(${modelIdentifier} SHARED
        model.cpp
        "$<TARGET_OBJECTS:fmu4cpp>")
target_include_directories(${modelIdentifier} PRIVATE ${PROJECT_SOURCE_DIR}/export/include)
target_compile_definitions(${modelIdentifier} PRIVATE FMU4CPP_MODEL_IDENTIFIER="${modelIdentifier}")

# Generate FMU
generateFMU("${modelIdentifier}")

set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/model.txt")
file(WRITE ${OUTPUT_FILE} "${modelIdentifier}")

#generateFMU("model" "${modelIdentifier}")
