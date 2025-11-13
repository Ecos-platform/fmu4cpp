
generateFMU("bouncing_ball"
        SOURCES
        "bouncing_ball.cpp"
        FMI_VERSIONS
        "fmi2;fmi3"
        WITH_SOURCES
        DOC_FOLDER
        "${CMAKE_CURRENT_SOURCE_DIR}/documentation"
)
