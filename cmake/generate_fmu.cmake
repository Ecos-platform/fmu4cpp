
function(generateFMU modelIdentifier fmiVersion resourceFolder)

    target_sources(${modelIdentifier} PRIVATE "$<TARGET_OBJECTS:fmu4cpp_base>")

    set(TARGET_PLATFORM)
    if (fmiVersion STREQUAL "fmi2")

        if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
            set(BITNESS 64)
        else ()
            set(BITNESS 32)
        endif ()

        if (WIN32)
            set(TARGET_PLATFORM win${BITNESS})
        elseif (APPLE)
            set(TARGET_PLATFORM darwin${BITNESS})
        else ()
            set(TARGET_PLATFORM linux${BITNESS})
        endif ()

        target_sources(${modelIdentifier} PRIVATE "$<TARGET_OBJECTS:fmu4cpp_fmi2>")
    elseif (fmiVersion STREQUAL "fmi3")

        set(TARGET_PLATFORM "x86")
        if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
            set(TARGET_PLATFORM "${TARGET_PLATFORM}_64")
        endif ()

        if (WIN32)
            set(TARGET_PLATFORM ${TARGET_PLATFORM}-windows)
        elseif (APPLE)
            set(TARGET_PLATFORM ${TARGET_PLATFORM}-darwin)
        else ()
            set(TARGET_PLATFORM ${TARGET_PLATFORM}-linux)
        endif ()

        target_sources(${modelIdentifier} PRIVATE "$<TARGET_OBJECTS:fmu4cpp_fmi3>")

    else ()
        message(FATAL_ERROR "Unknown FMI version: ${fmiVersion}. Supported versions are 'fmi2' and 'fmi3'.")
    endif ()

    target_include_directories("${modelIdentifier}" PRIVATE "${PROJECT_SOURCE_DIR}/export/include")
    target_compile_definitions("${modelIdentifier}" PRIVATE FMU4CPP_MODEL_IDENTIFIER="${modelIdentifier}")


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

    if (resourceFolder STREQUAL "")
        add_custom_command(TARGET ${modelIdentifier} POST_BUILD
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${modelIdentifier}"
                COMMAND ${CMAKE_COMMAND} -E tar "c" "${modelIdentifier}.fmu" --format=zip
                "${CMAKE_BINARY_DIR}/${modelIdentifier}/binaries"
                "${CMAKE_BINARY_DIR}/${modelIdentifier}/modelDescription.xml")

    else ()
        message("[generateFMU] Using resourceFolder=${resourceFolder} for model with identifier='${modelIdentifier}'")

        file(COPY "${resourceFolder}/" DESTINATION "${CMAKE_BINARY_DIR}/${modelIdentifier}/resources")

        add_custom_command(TARGET ${modelIdentifier} POST_BUILD
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${modelIdentifier}"
                COMMAND ${CMAKE_COMMAND} -E tar "c" "${modelIdentifier}.fmu" --format=zip
                "resources"
                "${CMAKE_BINARY_DIR}/${modelIdentifier}/binaries"
                "${CMAKE_BINARY_DIR}/${modelIdentifier}/modelDescription.xml")
    endif ()


endfunction()
