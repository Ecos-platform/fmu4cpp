
generateFMU(identity
        SOURCES
        "model.cpp"
        FMI_VERSIONS
        "fmi2;fmi3"
        RESOURCE_FOLDER
        "${PROJECT_SOURCE_DIR}/resources"
)
