
add_library(${modelIdentifier} SHARED
        identity.cpp
        "$<TARGET_OBJECTS:fmu4cpp>")
target_include_directories(identity PRIVATE ${PROJECT_SOURCE_DIR}/export)

set(outputDir "${CMAKE_BINARY_DIR}/bin/${TARGET_PLATFORM}")

if (WIN32)
    set_target_properties(${modelIdentifier}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${outputDir}"
            )
else ()
    set_target_properties(${modelIdentifier}
            PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${outputDir}"
            )
endif ()

add_custom_command(TARGET ${modelIdentifier} POST_BUILD
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMAND python "${PROJECT_SOURCE_DIR}/cmake/builder.py" ${modelIdentifier} "${outputDir}/$<TARGET_FILE_NAME:${modelIdentifier}>")
