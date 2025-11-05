
add_library(bouncing_ball OBJECT
        "bouncing_ball.cpp"
)

generateFMU("bouncing_ball" FMI_VERSIONS "fmi2" "fmi3")
