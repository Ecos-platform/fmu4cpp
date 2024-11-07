
function(generateFMU modelIdentifier)

    set(outputDir "$<1:${CMAKE_BINARY_DIR}/${modelIdentifier}/binaries/${TARGET_PLATFORM}>")

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
            COMMAND descriptionGenerator ${modelIdentifier} "${outputDir}/$<TARGET_FILE_NAME:${modelIdentifier}>")


    add_custom_command(TARGET ${modelIdentifier} POST_BUILD
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${modelIdentifier}"
            COMMAND ${CMAKE_COMMAND} -E tar "c" "${modelIdentifier}.fmu" --format=zip
            "${CMAKE_BINARY_DIR}/${modelIdentifier}/binaries"
            "${CMAKE_BINARY_DIR}/${modelIdentifier}/modelDescription.xml")
endfunction()
