# FMU4cpp (early prototype)

FMU4cpp is a CMake template repository that allows you to easily create FMUs compatible with [FMI 2.0](https://fmi-standard.org/downloads/)
using in C++.

FMU4cpp handles everything including generating the `modelDescription.xml`, 
and packaging of the content into an FMU archive.

Only tested on Windows.

### How do I get started?

1. Change the value of the `modelIdentifier` variable in `CMakeLists.txt` to something more appropriate.
2. Edit `model_info.json`.
3. Edit the content of `src/model.cpp`.
4. Build.

An FMU named `<modelIdentifier>.fmu` is now located in your build folder.

Such easy, such wow.

### Requirements
* C++17 compiler
* CMake >= 3.15
* Python3
