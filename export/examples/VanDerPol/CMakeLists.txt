
add_library(van_der_pol OBJECT
        "van_der_pol.cpp"
)

generateFMU(van_der_pol FMI_VERSIONS "fmi3")
