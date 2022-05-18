
include_directories(
        "${CMAKE_SOURCE_DIR}/export/include"
        "${CMAKE_SOURCE_DIR}/export/src"
)
link_libraries(Catch2::Catch2)

add_executable(basic_test basic_test.cpp "$<TARGET_OBJECTS:fmu4cpp>")
target_compile_definitions(basic_test PRIVATE FMU4CPP_MODEL_IDENTIFIER="basic_test")
add_test(NAME basic_test COMMAND basic_test)
