---
title: 'FMU4cpp: A GitHub template repository for creating Functional Mockup Units in C++'
tags:
  - FMI
  - FMI 2.0
  - FMU
  - Simulation and Modelling
  - CMake
  - GitHub Actions
authors:
  - name: Lars Ivar Hatledal
    orcid: 0000-0001-6436-7213
    affiliation: 1

affiliations:
 - name: Norwegian University of Science and Technology, Norway
   index: 1
date: 2 December 2024
bibliography: paper.bib
---

# Summary

FMU4cpp is a CMake template repository that acts as a framework for creating Functional Mockup Units (FMUs) compatible with the Functional Mockup Interface (FMI) standard [@blochwitz2011functional] in idiomatic C++. 
Currently, version 2.0 for Co-simulation [@blochwitz2012functional] is supported.
In short, an FMU is a component implementation the FMI standard. They are distributed as zipped archive consisting of shared libraries implementing a known 
C interface together with a _modelDescription.xml_ file detailing their capabilities and available variables. Additionally, model specific resources may be embedded.
The intention of FMU4cpp is to provide a streamlined way of generating such FMUs based on C++ source code.
To do so, the framework provides skeleton code for the user to implement, 
so that most of the low level details of the FMI standard is hidden. 
Furthermore, it handles generating the required XML as well as packaging the model according to the requirements. 
To further simplify model creation, the framework utilizes GitHub Actions in order to automatically build and package cross-platform FMUs.


# Statement of need

A survey of existing open-source software that facilitates exporting FMUs from source-code is shown in Table 1.
The non-native solutions listed here provides similar capabilities, however the native solutions require a substantial expert knowledge 
and users are left with the task of generating metadata and final packaging. 
Thus, FMU4cpp provides a significant usability improvement over these alternatives.


Table 1: Survey of existing open-source FMI exporters.

|               Name                | Language | Most recent Update |
|:---------------------------------:|:--------:|:------------------:|
|         FMU4j   [@fmu4j]          |   Java   |        2022        |
|        JavaFMI  [@javafmi]        |   Java   |        2021        |
|        cppfmu   [@cppfmu]         |   C++    |        2024        |
| Reference FMUs [@reference_fmus]  |    C     |        2024        |
| PythonFMU [@hatledal2020enabling] |  Python  |        2024        |
|  unifmu [@legaard2021universal]   | Multiple |        2024        |

# Future of FMU4cpp

With FMI 3.0 [@junghanns2021functional] released in 2022, an obvious and planned enhancement to FMU4cpp is to support this newer version of the standard.

# References