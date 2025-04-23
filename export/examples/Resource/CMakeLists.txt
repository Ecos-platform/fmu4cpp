
add_library(resource SHARED
        "resource.cpp"
)

generateFMU("resource" "fmi2" "${CMAKE_CURRENT_SOURCE_DIR}/resources")
