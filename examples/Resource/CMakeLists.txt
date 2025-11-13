
generateFMU("resource"
        SOURCES
        "resource.cpp"
        FMI_VERSIONS
        "fmi2;fmi3"
        RESOURCE_FOLDER
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
)
