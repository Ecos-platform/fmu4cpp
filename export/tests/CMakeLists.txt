
include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../src"
)

function(make_test modelIdentifier sources)
    add_executable(${modelIdentifier} ${sources} "$<TARGET_OBJECTS:fmu4cpp>")
    target_link_libraries(${modelIdentifier} PUBLIC Catch2::Catch2WithMain)
    target_compile_definitions(${modelIdentifier} PRIVATE FMU4CPP_MODEL_IDENTIFIER="${modelIdentifier}")
    add_test(NAME ${modelIdentifier} COMMAND ${modelIdentifier})
endfunction()

make_test(basic_test basic_test.cpp)
