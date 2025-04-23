
add_library(bouncing_ball SHARED
        "bouncing_ball.cpp"
)
target_include_directories(bouncing_ball PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../external"
)

generateFMU("bouncing_ball" "fmi3" "")
