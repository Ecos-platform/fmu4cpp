
add_library(bouncing_ball SHARED
        bouncing_ball.cpp
        "$<TARGET_OBJECTS:fmu4cpp>"
)
target_include_directories(bouncing_ball PRIVATE "../../include")
target_compile_definitions(bouncing_ball PRIVATE FMU4CPP_MODEL_IDENTIFIER="bouncing_ball")

generateFMU("bouncing_ball")