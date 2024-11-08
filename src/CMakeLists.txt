

add_library(${modelIdentifier} SHARED
        "model.cpp"
)

# Generate FMU
generateFMU("${modelIdentifier}")

set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/model.txt")
file(WRITE ${OUTPUT_FILE} "${modelIdentifier}")
