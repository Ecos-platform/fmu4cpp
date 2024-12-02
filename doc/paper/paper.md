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

FMU4cpp is a CMake template repository that acts as a framework for creating Functional Mockup Units (FMUs) compatiable with the Functional Mockup Interface (FMI) standard [@blochwitz2011functional] in idiomatic C++. 
Currently, version 2.0 for Co-simulation [@blochwitz2012functional] is supported.
The framework provides skeleton code for the user to implement, 
so that most of the low level details of the FMI standard is hidden. 
Furthermore, it handles generating the required XML as well as packaging the model according to the requirements. To further easy model creation, 
the framework utilizes GitHub Actions in order to automatically build and package cross-platform FMUs.
Frameworks with similar capabilities exists for Python and Java, however the native alternatives does not automatically 
handle XML generation and packaging, nor automatic cross-platform compilation. 
Thus, FMU4cpp provides a significant usability improvement over these alternatives. 

# Statement of need

Table 1: Survey of existing open-source FMI exporters.

|               Name                | Language | Most recent Update |
|:---------------------------------:|:--------:|:------------------:|
|         FMU4j   [@fmu4j]          |   Java   |        2022        |
|        JavaFMI  [@javafmi]        |   Java   |        2021        |
|        cppfmu   [@cppfmu]         |   C++    |        2024        |
| Reference FMUs [@reference_fmus]  |    C     |        2024        |
| PythonFMU [@hatledal2020enabling] |  Python  |        2024        |

# Future of FMU4cpp

With FMI 3.0 [@junghanns2021functional] released in 2022, an obvious enhancement to FMU4cpp is to support this newer version of the standard.

# References