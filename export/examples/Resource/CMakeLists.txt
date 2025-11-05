
add_library(resource OBJECT
        "resource.cpp"
)

generateFMU("resource" FMI_VERSIONS "fmi2" RESOURCE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/resources")
