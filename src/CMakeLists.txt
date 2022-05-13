
set(target_name identity)

add_library(${target_name} SHARED
        identity.cpp
        "$<TARGET_OBJECTS:fmu4cpp>")
target_include_directories(identity PRIVATE ${PROJECT_SOURCE_DIR}/export)

set(outputDir "${CMAKE_BINARY_DIR}/bin/${TARGET_PLATFORM}")

if (WIN32)
    set_target_properties(${target_name}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${outputDir}"
            )
else ()
    set_target_properties(${target_name}
            PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${outputDir}"
            )
endif ()

add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND python "${PROJECT_SOURCE_DIR}/cmake/builder.py" "${outputDir}/$<TARGET_FILE_NAME:${target_name}>")
