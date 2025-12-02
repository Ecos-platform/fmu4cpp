
generateFMU(onnx_tracker
        SOURCES "tracking.cpp"
        RESOURCE_FOLDER
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        LINK_TARGETS ${OpenCV_LIBRARIES}
        FMI_VERSIONS
        "fmi3"
)
