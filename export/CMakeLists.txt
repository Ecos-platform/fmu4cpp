add_subdirectory(src)

add_executable(descriptionGenerator descriptionGenerator.cpp)
if (UNIX)
    target_link_libraries(descriptionGenerator PRIVATE dl)
endif()
