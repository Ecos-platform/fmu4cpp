
add_library(fmu4cpp
        fmu4cpp/fmi2.cpp
        fmu4cpp/fmu_base.cpp)
target_compile_features(fmu4cpp PUBLIC "cxx_std_17")
target_include_directories(fmu4cpp
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        )