
function(make_test modelIdentifier sources)
    add_executable(${modelIdentifier} ${sources} "$<TARGET_OBJECTS:fmu4cpp>")
    add_test(NAME ${modelIdentifier} COMMAND ${modelIdentifier})
    target_link_libraries(${modelIdentifier} PUBLIC Catch2::Catch2WithMain)
    target_compile_definitions(${modelIdentifier}
            PRIVATE
            FMU4CPP_MODEL_IDENTIFIER="${modelIdentifier}"
            TEST_CASE_RESOURCE_LOCATION="${CMAKE_CURRENT_SOURCE_DIR}/resources"
    )
    target_include_directories(${modelIdentifier}
            PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/../include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    )

endfunction()

make_test(basic_test basic_test.cpp)
make_test(test_resource test_resource.cpp)
