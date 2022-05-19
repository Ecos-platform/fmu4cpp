
add_library(${modelIdentifier} SHARED
        model.cpp
        "$<TARGET_OBJECTS:fmu4cpp>")
target_include_directories(${modelIdentifier} PRIVATE ${PROJECT_SOURCE_DIR}/export/include)
target_compile_definitions(${modelIdentifier} PRIVATE FMU4CPP_MODEL_IDENTIFIER="${modelIdentifier}")

set(outputDir "$<1:${CMAKE_BINARY_DIR}/binaries/${TARGET_PLATFORM}>")

if (WIN32)
    set_target_properties(${modelIdentifier}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${outputDir}"
            )
else ()
    set_target_properties(${modelIdentifier}
            PROPERTIES
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${outputDir}"
            )
endif ()

# Generate modelDescription.xml
add_custom_command(TARGET ${modelIdentifier} POST_BUILD
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMAND descriptionGenerator "${outputDir}/$<TARGET_FILE_NAME:${modelIdentifier}>")

function(generateFMU name)
    add_custom_command(TARGET ${modelIdentifier} POST_BUILD
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMAND ${CMAKE_COMMAND} -E tar "c" "${name}.fmu" --format=zip
            "${CMAKE_BINARY_DIR}/binaries"
            "${CMAKE_BINARY_DIR}/modelDescription.xml")
endfunction()

# Generate FMU
generateFMU("${modelIdentifier}")

# Make a copy for testing purposes
generateFMU("model")
