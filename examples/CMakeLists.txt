
add_executable(sine sine.cpp "$<TARGET_OBJECTS:fmu4cpp>")
target_include_directories(sine PRIVATE ${CMAKE_SOURCE_DIR}/export)

add_library(libsine SHARED sine.cpp "$<TARGET_OBJECTS:fmu4cpp>")
target_include_directories(libsine PRIVATE ${CMAKE_SOURCE_DIR}/export)
