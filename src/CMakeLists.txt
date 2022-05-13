
add_library(identity SHARED
        identity.cpp
        "$<TARGET_OBJECTS:fmu4cpp>")
target_include_directories(identity PRIVATE ${PROJECT_SOURCE_DIR}/export)
