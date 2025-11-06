

# Generate FMU
generateFMU("${modelIdentifier}"
        SOURCES
        "model.cpp"
        FMI_VERSIONS
        ${fmi_version}
        RESOURCE_FOLDER
        "${PROJECT_SOURCE_DIR}/resources"
)

# write the identifier to file so that the CI process knows what to package
set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/model.txt")
file(WRITE ${OUTPUT_FILE} "${fmi_version}/${modelIdentifier}")
