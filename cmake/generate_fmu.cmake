
set(_fmu4cpp_cmake_dir "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(_fmu4cpp_root "${_fmu4cpp_cmake_dir}/.." ABSOLUTE)
set(_fmu4cpp_root "${_fmu4cpp_root}" CACHE INTERNAL "")


function(generateFMU modelIdentifier)

    set(options WITH_SOURCES)
    set(oneValueArgs RESOURCE_FOLDER DOC_FOLDER DESTINATION)
    set(multiValueArgs FMI_VERSIONS SOURCES INCLUDE_DIRS LINK_TARGETS COMPILE_DEFINITIONS)
    cmake_parse_arguments(FMU "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(fmuResultDir "${CMAKE_BINARY_DIR}/models")
    if (NOT EXISTS "${fmuResultDir}")
        file(MAKE_DIRECTORY "${fmuResultDir}")
    endif ()

    set(generatedSourcesDir "${CMAKE_BINARY_DIR}/generated")
    if (NOT EXISTS "${generatedSourcesDir}")
        file(MAKE_DIRECTORY "${generatedSourcesDir}")
    endif ()

    if (NOT FMU_RESOURCE_FOLDER)
        set(FMU_RESOURCE_FOLDER "")
    endif ()

    if (NOT FMU_DOC_FOLDER)
        set(FMU_DOC_FOLDER "")
    endif ()

    # Require at least one fmi version; default to fmi2 if none provided
    if (NOT FMU_FMI_VERSIONS)
        message(FATAL_ERROR "generateFMU requires at least one FMI version to be specified")
    endif ()

    # require sources list (user now passes sources instead of an object library)
    if (NOT FMU_SOURCES)
        message(FATAL_ERROR "generateFMU requires SOURCES to be provided")
    endif ()


    foreach (fmiVersion IN LISTS FMU_FMI_VERSIONS)

        _getTargetPlatform(${fmiVersion})

        set(fmuOutputDir "${fmuResultDir}/${fmiVersion}")
        set(modelOutputDir "${fmuOutputDir}/${modelIdentifier}")
        set(binaryOutputDir "$<1:${modelOutputDir}/binaries/${TARGET_PLATFORM}>")

        set(versionTarget "${modelIdentifier}_${fmiVersion}")

        set(FMU4CPP_MODEL_IDENTIFIER "${versionTarget}")
        set(VERSION_OBJECTS "${generatedSourcesDir}/fmu4cpp/model_identifier_${versionTarget}.cpp")
        configure_file(
                "${_fmu4cpp_root}/export/src/fmu4cpp/model_identifier.cpp.in"
                "${VERSION_OBJECTS}"
                @ONLY
        )

        set(VERSIONS_DEFS "")
        if (fmiVersion STREQUAL "fmi2")
            list(APPEND VERSION_DEFS "FMI2")
            target_compile_definitions(fmu4cpp_fmi2 PUBLIC "FMI2")
            list(APPEND VERSION_OBJECTS "$<TARGET_OBJECTS:fmu4cpp_fmi2>")
        elseif (fmiVersion STREQUAL "fmi3")
            list(APPEND VERSION_DEFS "FMI3")
            target_compile_definitions(fmu4cpp_fmi3 PUBLIC "FMI3")
            list(APPEND VERSION_OBJECTS "$<TARGET_OBJECTS:fmu4cpp_fmi3>")
        else ()
            message(FATAL_ERROR "Unknown FMI version: ${fmiVersion}. Supported versions are 'fmi2' and 'fmi3'.")
        endif ()

        add_library(${versionTarget} SHARED
                "$<TARGET_OBJECTS:fmu4cpp_base>"
                "${FMU_SOURCES}"
                "${VERSION_OBJECTS}"
        )
        target_include_directories(${versionTarget}
                PRIVATE
                "${_fmu4cpp_root}/export/include"
                "${FMU_INCLUDE_DIRS}"
        )

        target_compile_definitions(${versionTarget}
                PUBLIC
                ${VERSION_DEFS}
                ${FMU_COMPILE_DEFINITIONS}
        )


        if (FMU_LINK_TARGETS)
            target_link_libraries(${versionTarget} PRIVATE ${FMU_LINK_TARGETS})
            _bundle_link_libraries()
        endif ()

        if (FMU_WITH_SOURCES)
            if (fmiVersion STREQUAL "fmi2")
                message(WARNING "[generateFMU-fmi2] FMU_WITH_SOURCES is not supported for fmi2; skipping source inclusion for model '${modelIdentifier}'")
            else ()
                _include_sources_in_fmu()
            endif ()
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

        _package_fmu()

    endforeach ()

endfunction()

macro(_package_fmu)
    set(TAR_INPUTS "${modelOutputDir}/binaries" "${modelOutputDir}/modelDescription.xml")
    if (FMU_WITH_SOURCES AND fmiVersion STREQUAL "fmi3")
        list(APPEND TAR_INPUTS "${modelOutputDir}/sources" "${modelOutputDir}/sources/buildDescription.xml")
    endif ()

    if (NOT FMU_DOC_FOLDER STREQUAL "")
        message("[generateFMU-${fmiVersion}] Using documentation folder=${FMU_DOC_FOLDER} for model '${modelIdentifier}'")
        file(COPY "${FMU_DOC_FOLDER}/" DESTINATION "${modelOutputDir}/documentation")
        list(PREPEND TAR_INPUTS "documentation")
    endif ()

    if (NOT FMU_RESOURCE_FOLDER STREQUAL "")
        message("[generateFMU-${fmiVersion}] Using resource folder=${FMU_RESOURCE_FOLDER} for model '${modelIdentifier}'")
        file(COPY "${FMU_RESOURCE_FOLDER}/" DESTINATION "${modelOutputDir}/resources")
        list(PREPEND TAR_INPUTS "resources")
    endif ()

    if (FMU_DESTINATION)
        set(FMU_DEST_DIR "${FMU_DESTINATION}/${fmiVersion}")
        file(MAKE_DIRECTORY "${FMU_DEST_DIR}")
        set(FMU_DESTINATION_ "${FMU_DEST_DIR}/${modelIdentifier}.fmu")
    else ()
        set(FMU_DESTINATION_ "${modelIdentifier}.fmu")
    endif ()
    add_custom_command(TARGET ${versionTarget} POST_BUILD
            WORKING_DIRECTORY "${modelOutputDir}"
            COMMAND ${CMAKE_COMMAND} -E echo "[generateFMU-${fmiVersion}] Packaging ${modelIdentifier}.fmu in ${FMU_DESTINATION_}"
            COMMAND ${CMAKE_COMMAND} -E tar c "${FMU_DESTINATION_}" --format=zip ${TAR_INPUTS}
    )
endmacro()

macro(_bundle_link_libraries)
    target_link_libraries(${versionTarget} PRIVATE ${FMU_LINK_TARGETS})

    foreach (dep IN LISTS FMU_LINK_TARGETS)
        if (TARGET ${dep})
            get_target_property(target_type ${dep} TYPE)
            if (NOT "${target_type}" STREQUAL "INTERFACE_LIBRARY")
                add_custom_command(TARGET ${versionTarget} POST_BUILD
                        WORKING_DIRECTORY "${modelOutputDir}"
                        COMMAND ${CMAKE_COMMAND} -E echo "[generateFMU-${fmiVersion}] Copying runtime of ${dep} to ${binaryOutputDir}"
                        COMMAND ${CMAKE_COMMAND} -E make_directory "${binaryOutputDir}"
                        # copy the target's runtime file (dll/so/dylib) into the binaries folder
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        $<TARGET_FILE:${dep}>
                        "${binaryOutputDir}/$<TARGET_FILE_NAME:${dep}>"
                )
            endif ()
        endif ()
    endforeach ()
endmacro()

macro(_include_sources_in_fmu)

    message("[generateFMU-${fmiVersion}] Including sources in FMU for model '${modelIdentifier}'")

    set(VERSION_SOURCES ${VERSION_OBJECTS})
    file(MAKE_DIRECTORY "${modelOutputDir}/sources")
    file(COPY "${generatedSourcesDir}/fmu4cpp/lib_info.cpp" DESTINATION "${modelOutputDir}/sources/fmu4cpp/")
    file(COPY ${_fmu4cpp_root}/export/src/fmu4cpp DESTINATION "${modelOutputDir}/sources/")
    file(REMOVE_RECURSE "${modelOutputDir}/sources/fmu4cpp/fmi2") # remove fmi2 sources
    file(COPY ${_fmu4cpp_root}/export/include/fmu4cpp DESTINATION "${modelOutputDir}/sources/")

    foreach (dir IN LISTS FMU_INCLUDE_DIRS)
        file(COPY "${dir}/" DESTINATION "${modelOutputDir}/sources/")
    endforeach ()

    foreach (s IN LISTS FMU_SOURCES VERSION_SOURCES)
        if (NOT "${s}" MATCHES "^\\$<") # skip generator expressions
            if (IS_ABSOLUTE "${s}")
                set(abs "${s}")
            else ()
                get_filename_component(abs "${CMAKE_CURRENT_SOURCE_DIR}/${s}" ABSOLUTE)
            endif ()
            file(COPY "${abs}" DESTINATION "${modelOutputDir}/sources/")
        endif ()
    endforeach ()

    # glob .cpp files in ${modelOutputDir}/sources/
    set(SOURCE_SET "")
    file(GLOB_RECURSE COPIED_CPP_FILES "${modelOutputDir}/sources/*.cpp")
    foreach (cpp_file IN LISTS COPIED_CPP_FILES)
        # get relative path to sources dir
        file(RELATIVE_PATH rel_path "${modelOutputDir}/sources" "${cpp_file}")
        set(SOURCE_SET "${SOURCE_SET}\t\t\t<SourceFile name=\"${rel_path}\"/>\n")
    endforeach ()

    #write buildDescription.xml
    file(WRITE "${modelOutputDir}/sources/buildDescription.xml"
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<fmiBuildDescription fmiVersion=\"3.0\">\n"
            "\t<BuildConfiguration modelIdentifier=\"${FMU4CPP_MODEL_IDENTIFIER}\">\n"
            "\t\t<SourceFileSet language=\"C++17\">\n"
            ${SOURCE_SET}
            "\t\t\t<PreprocessorDefinition name=\"FMI3\"/>\n"
            "\t\t\t<IncludeDirectory name=\"include/\"/>\n"
            "\t\t</SourceFileSet>\n"
            "\t</BuildConfiguration>\n"
            "</fmiBuildDescription>\n"
    )
endmacro()

function(_getTargetPlatform fmiVersion)
    set(_target "")
    if (fmiVersion STREQUAL "fmi2")

        if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
            set(BITNESS 64)
        else ()
            set(BITNESS 32)
        endif ()

        if (WIN32)
            set(_target win${BITNESS})
        elseif (APPLE)
            set(_target darwin${BITNESS})
        else ()
            set(_target linux${BITNESS})
        endif ()

    elseif (fmiVersion STREQUAL "fmi3")

        set(_target "x86")
        if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
            set(_target "${_target}_64")
        endif ()

        if (WIN32)
            set(_target ${_target}-windows)
        elseif (APPLE)
            set(_target ${_target}-darwin)
        else ()
            set(_target ${_target}-linux)
        endif ()

    else ()
        message(FATAL_ERROR "Unknown FMI version: ${fmiVersion}. Supported versions are 'fmi2' and 'fmi3'.")
    endif ()

    set(TARGET_PLATFORM ${_target} PARENT_SCOPE)

endfunction()
