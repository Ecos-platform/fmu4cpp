
add_library(simple_pendulum SHARED
        "simple_pendulum.cpp"
)

generateFMU(simple_pendulum "fmi2")
