
#ifndef FMU4CPP_STATUS_HPP
#define FMU4CPP_STATUS_HPP

typedef enum {
    fmiOK,
    fmiWarning,
    fmiDiscard,
    fmiError,
    fmiFatal,
    fmiStatusUnknown // Optional: for error handling or undefined states
} fmiStatus;

#endif //FMU4CPP_STATUS_HPP
