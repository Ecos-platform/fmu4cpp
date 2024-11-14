

add_library(${modelIdentifier} SHARED
        "model.cpp"
)

# Generate FMU
generateFMU("${modelIdentifier}" "${PROJECT_SOURCE_DIR}/resources")

set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/model.txt")
file(WRITE ${OUTPUT_FILE} "${modelIdentifier}")
