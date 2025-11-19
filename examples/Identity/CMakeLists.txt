
generateFMU(identity
        SOURCES
        "model.cpp"
        INCLUDE_DIRS
        "${PROJECT_SOURCE_DIR}/export/tests/models"
        FMI_VERSIONS
        "fmi2;fmi3"
)
