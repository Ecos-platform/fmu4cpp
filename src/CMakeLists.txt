
# library must have SHARED linkage
add_library(${modelIdentifier} SHARED
        "model.cpp"
)

# Generate FMU
generateFMU("${modelIdentifier}" "${PROJECT_SOURCE_DIR}/resources")

# write the identifier to file so that the CI process knows what to package
set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/model.txt")
file(WRITE ${OUTPUT_FILE} "${modelIdentifier}")
