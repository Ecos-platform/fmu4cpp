
add_library(van_der_pol SHARED
        "van_der_pol.cpp"
)

generateFMU(van_der_pol "fmi3")
