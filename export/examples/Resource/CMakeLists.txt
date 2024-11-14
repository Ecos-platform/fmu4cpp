
add_library(resource SHARED
        "resource.cpp"
)

generateFMU("resource" "${CMAKE_CURRENT_SOURCE_DIR}/resources")
