
function(make_test name sources)
    add_executable(${name} ${sources} "$<TARGET_OBJECTS:fmu4cpp>")
    add_test(NAME ${name} COMMAND ${modelIdentifier})
    target_link_libraries(${name} PUBLIC Catch2::Catch2WithMain)
    target_compile_definitions(${name}
            PRIVATE
            FMU4CPP_MODEL_IDENTIFIER="${name}"
            TEST_CASE_RESOURCE_LOCATION="${CMAKE_CURRENT_SOURCE_DIR}/resources"
    )
    target_include_directories(${name}
            PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/../include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    )

endfunction()

make_test(basic_test basic_test.cpp)
make_test(test_resource test_resource.cpp)

add_executable(bouncingball_test bouncingball_test.cpp)
target_link_libraries(bouncingball_test PRIVATE Catch2::Catch2WithMain fmi4c)
