
generateFMU(onnx_tracker
        SOURCES "tracking.cpp"
        RESOURCE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/data"
        LINK_TARGETS opencv_core opencv_dnn
        FMI_VERSIONS
        "fmi3"
)
