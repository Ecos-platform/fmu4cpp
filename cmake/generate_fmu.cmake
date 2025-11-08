
set(_fmu4cpp_cmake_dir "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(_fmu4cpp_root "${_fmu4cpp_cmake_dir}/.." ABSOLUTE)

function(generateFMU modelIdentifier)

    set(options)
    set(oneValueArgs RESOURCE_FOLDER)
    set(multiValueArgs FMI_VERSIONS SOURCES LINK_TARGETS COMPILE_DEFINITIONS)
    cmake_parse_arguments(FMU "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT FMU_RESOURCE_FOLDER)
        set(FMU_RESOURCE_FOLDER "")
    endif ()

    # Require at least one fmi version; default to fmi2 if none provided
    if (NOT FMU_FMI_VERSIONS)
        message(FATAL_ERROR "generateFMU requires at least one FMI version to be specified")
    endif ()

    # require sources list (user now passes sources instead of an object library)
    if (NOT FMU_SOURCES)
        message(FATAL_ERROR "generateFMU requires SOURCES to be provided")
    endif ()


    # common object files from the base target
    set(COMMON_OBJECTS "$<TARGET_OBJECTS:fmu4cpp_base>")

    # create an internal object library from provided sources (if not already created)
    set(model_objects_target "${modelIdentifier}_fmu_objects")
    if (NOT TARGET ${model_objects_target})
        add_library(${model_objects_target} OBJECT ${FMU_SOURCES})
        target_include_directories(${model_objects_target} PUBLIC "${PROJECT_SOURCE_DIR}/export/include")
        if (FMU_LINK_TARGETS)
            target_link_libraries(${model_objects_target} PRIVATE ${FMU_LINK_TARGETS})
        endif ()
        # apply user-provided compile definitions to the object target
        if (FMU_COMPILE_DEFINITIONS)
            target_compile_definitions(${model_objects_target} PRIVATE ${FMU_COMPILE_DEFINITIONS})
        endif ()
        set_target_properties(${model_objects_target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif ()

    foreach (fmiVersion IN LISTS FMU_FMI_VERSIONS)

        # versioned shared library target built from object libraries
        set(versionTarget "${modelIdentifier}_${fmiVersion}")

        set(FMU4CPP_MODEL_IDENTIFIER "${versionTarget}")
        set(model_identifier_src "${generatedSourcesDir}/fmu4cpp/model_identifier_${versionTarget}.cpp")
        configure_file(
                "${_fmu4cpp_root}/export/src/fmu4cpp/model_identifier.cpp.in"
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


        set(fmuOutputDir "${fmuResultDir}/${fmiVersion}")
        set(modelOutputDir "${fmuOutputDir}/${modelIdentifier}")
        set(binaryOutputDir "$<1:${modelOutputDir}/binaries/${TARGET_PLATFORM}>")


        add_library(${versionTarget} SHARED
                ${COMMON_OBJECTS}
                "$<TARGET_OBJECTS:${model_objects_target}>"
                ${VERSION_OBJECTS}
        )
        target_include_directories(${versionTarget} PRIVATE "${PROJECT_SOURCE_DIR}/export/include")
        target_compile_definitions(${versionTarget} PRIVATE ${VERSION_DEFS})

        # link user-provided link targets (must be propagated to the final shared lib)
        if (FMU_LINK_TARGETS)
            target_link_libraries(${versionTarget} PRIVATE ${FMU_LINK_TARGETS})

            foreach(dep IN LISTS FMU_LINK_TARGETS)
                if (TARGET ${dep})
                    add_custom_command(TARGET ${versionTarget} POST_BUILD
                            WORKING_DIRECTORY "${modelOutputDir}"
                            COMMAND ${CMAKE_COMMAND} -E echo "[generateFMU-${fmiVersion}] Copying runtime of ${dep} to ${binaryOutputDir}"
                            COMMAND ${CMAKE_COMMAND} -E make_directory "${binaryOutputDir}"
                            # copy the target's runtime file (dll/so/dylib) into the binaries folder
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            $<TARGET_FILE:${dep}>
                            "${binaryOutputDir}/$<TARGET_FILE_NAME:${dep}>"
                    )
                endif()
            endforeach()
        endif ()

        # if user provided compile definitions, also ensure final target sees them (optional)
        if (FMU_COMPILE_DEFINITIONS)
            target_compile_definitions(${model_objects_target} PRIVATE ${FMU_COMPILE_DEFINITIONS})
            target_compile_definitions(${versionTarget} PRIVATE ${FMU_COMPILE_DEFINITIONS})
        endif ()

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
                WORKING_DIRECTORY "${binaryOutputDir}"
                COMMAND ${CMAKE_COMMAND} -E echo "[generateFMU-${fmiVersion}] Generating modelDescription.xml for model '${modelIdentifier}'"
                COMMAND descriptionGenerator "$<TARGET_FILE_NAME:${versionTarget}>")

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
