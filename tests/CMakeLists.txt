
include_directories(
        "${CMAKE_SOURCE_DIR}/export"
        "${CMAKE_SOURCE_DIR}/fmi_headers"
)
link_libraries(Catch2::Catch2)

add_executable(basic_test basic_test.cpp "$<TARGET_OBJECTS:fmu4cpp>")
add_test(NAME basic_test COMMAND basic_test)