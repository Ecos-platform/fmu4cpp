# FMU4cpp (early prototype)

FMU4cpp is a CMake template repository that allows you to easily create FMUs written in C++.

FMU4cpp handles everything including generating the `modelDescription.xml`, and packaging of the content into an FMU archive.

### How do I get started?

1. Change the value of the `modelIdentifier` variable in `CMakeLists.txt` to something more appropriate.
2. Edit `model_info.json`.
3. Edit the content of `src/model.cpp`
4. Build

An FMU named `<modelIdentifier>.fmu` is now located in your build folder.

Such easy, such wow.
