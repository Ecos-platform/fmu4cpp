
function(generateFMU modelIdentifier)

    set(options)
    set(oneValueArgs RESOURCE_FOLDER)
    set(multiValueArgs FMI_VERSIONS)
    cmake_parse_arguments(FMU "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT FMU_RESOURCE_FOLDER)
        set(FMU_RESOURCE_FOLDER "")
    endif ()

    # Require at least one fmi version; default to fmi2 if none provided
    if (NOT FMU_FMI_VERSIONS)
        message(FATAL_ERROR "generateFMU requires at least one FMI version to be specified")
    endif ()

    # Expect the user to provide modelIdentifier as an OBJECT library.
    # We will build per-version shared libraries from those object files.
    set(COMMON_OBJECTS "$<TARGET_OBJECTS:fmu4cpp_base>")
    target_include_directories(${modelIdentifier} PUBLIC "${PROJECT_SOURCE_DIR}/export/include")
    set_target_properties(${modelIdentifier} PROPERTIES POSITION_INDEPENDENT_CODE ON)

    foreach (fmiVersion IN LISTS FMU_FMI_VERSIONS)

        # versioned shared library target built from object libraries
        set(versionTarget "${modelIdentifier}_${fmiVersion}")

        set(FMU4CPP_MODEL_IDENTIFIER "${versionTarget}")
        set(model_identifier_src "${generatedSourcesDir}/fmu4cpp/model_identifier_${versionTarget}.cpp")
        configure_file(
                "${PROJECT_SOURCE_DIR}/export/src/fmu4cpp/model_identifier.cpp.in"
                "${model_identifier_src}"
                @ONLY
        )

        set(VERSION_OBJECTS "${model_identifier_src}")

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

            list(APPEND VERSION_OBJECTS "$<TARGET_OBJECTS:fmu4cpp_fmi2>")
            set(VERSION_DEFS FMI2)

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

            list(APPEND VERSION_OBJECTS "$<TARGET_OBJECTS:fmu4cpp_fmi3>")
            set(VERSION_DEFS FMI3)

        else ()
            message(FATAL_ERROR "Unknown FMI version: ${fmiVersion}. Supported versions are 'fmi2' and 'fmi3'.")
        endif ()


        set(fmuOutputDir "${CMAKE_BINARY_DIR}/${fmiVersion}")
        set(modelOutputDir "${fmuOutputDir}/${modelIdentifier}")
        set(binaryOutputDir "$<1:${modelOutputDir}/binaries/${TARGET_PLATFORM}>")


        add_library(${versionTarget} SHARED
                ${COMMON_OBJECTS}
                "$<TARGET_OBJECTS:${modelIdentifier}>"
                ${VERSION_OBJECTS}
        )
        target_include_directories(${versionTarget} PRIVATE "${PROJECT_SOURCE_DIR}/export/include")
        target_compile_definitions(${versionTarget} PRIVATE ${VERSION_DEFS})


        if (WIN32)
            set_target_properties(${versionTarget}
                    PROPERTIES
                    RUNTIME_OUTPUT_DIRECTORY "${binaryOutputDir}"
            )
        else ()
            set_target_properties(${versionTarget}
                    PROPERTIES
                    PREFIX ""
                    LIBRARY_OUTPUT_DIRECTORY "${binaryOutputDir}"
            )
        endif ()

        # Generate modelDescription.xml
        add_custom_command(TARGET ${versionTarget} POST_BUILD
                WORKING_DIRECTORY "${fmuOutputDir}"
                COMMAND ${CMAKE_COMMAND} -E echo "[generateFMU-${fmiVersion}] Generating modelDescription.xml for model '${modelIdentifier}'"
                COMMAND descriptionGenerator ${modelIdentifier} "${binaryOutputDir}/$<TARGET_FILE_NAME:${versionTarget}>")

        # Package FMU
        set(TAR_INPUTS "${modelOutputDir}/binaries" "${modelOutputDir}/modelDescription.xml")
        if (NOT FMU_RESOURCE_FOLDER STREQUAL "")
            message("[generateFMU-${fmiVersion}] Using resourceFolder=${FMU_RESOURCE_FOLDER} for model '${modelIdentifier}'")
            file(COPY "${FMU_RESOURCE_FOLDER}/" DESTINATION "${modelOutputDir}/resources")
            list(PREPEND TAR_INPUTS "resources")
        endif ()

        add_custom_command(TARGET ${versionTarget} POST_BUILD
                WORKING_DIRECTORY "${modelOutputDir}"
                COMMAND ${CMAKE_COMMAND} -E echo "[generateFMU-${fmiVersion}] Packaging ${modelIdentifier}.fmu in ${modelOutputDir}"
                COMMAND ${CMAKE_COMMAND} -E tar c "${modelIdentifier}.fmu" --format=zip ${TAR_INPUTS}
        )

    endforeach ()

endfunction()
