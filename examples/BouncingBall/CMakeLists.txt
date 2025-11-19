
generateFMU("bouncing_ball"
        SOURCES
        "bouncing_ball.cpp"
        INCLUDE_DIRS
        "${PROJECT_SOURCE_DIR}/export/tests/models"
        FMI_VERSIONS
        "fmi2;fmi3"
        WITH_SOURCES
        DOC_FOLDER
        "${CMAKE_CURRENT_SOURCE_DIR}/documentation"
)
