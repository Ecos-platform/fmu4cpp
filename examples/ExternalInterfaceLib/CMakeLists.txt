
include(FetchContent)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

generateFMU("external_interface_lib"
        SOURCES
        "external_interface_lib.cpp"
        INCLUDE_DIRS
        "${nlohmann_json_SOURCE_DIR}/include"
        FMI_VERSIONS
        "fmi2;fmi3"
        WITH_SOURCES
)
