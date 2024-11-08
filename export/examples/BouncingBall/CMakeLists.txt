
add_library(bouncing_ball SHARED
        "bouncing_ball.cpp"
)

generateFMU("bouncing_ball")
