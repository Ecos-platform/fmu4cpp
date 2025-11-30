
set(MODEL_PATH "${CMAKE_BINARY_DIR}/models/spring_mass_damper")

generateFMU("mass"
        SOURCES
        "mass.cpp"
        FMI_VERSIONS
        "fmi2;fmi3"
        WITH_SOURCES
        DESTINATION
        "${MODEL_PATH}"
)

generateFMU("spring"
        SOURCES
        "spring.cpp"
        FMI_VERSIONS
        "fmi2;fmi3"
        WITH_SOURCES
        DESTINATION
        "${MODEL_PATH}"
)

generateFMU("damper"
        SOURCES
        "damper.cpp"
        FMI_VERSIONS
        "fmi2;fmi3"
        WITH_SOURCES
        DESTINATION
        "${MODEL_PATH}"
)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/SystemStructure.ssd.in"
        "${MODEL_PATH}/SystemStructure.ssd"
        @ONLY
)

file(COPY "ChartConfig.xml" DESTINATION "${MODEL_PATH}")
file(COPY "simulate_with_ecos.bat" DESTINATION "${MODEL_PATH}")
file(COPY "simulate_with_ecos.sh" DESTINATION "${MODEL_PATH}")
