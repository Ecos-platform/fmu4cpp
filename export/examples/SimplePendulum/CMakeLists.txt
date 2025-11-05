
add_library(simple_pendulum OBJECT
        "simple_pendulum.cpp"
)

generateFMU(simple_pendulum FMI_VERSIONS "fmi3")
