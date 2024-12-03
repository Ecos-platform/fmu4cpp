
#ifndef FMU4CPP_TEMPLATE_LIB_INFO_HPP
#define FMU4CPP_TEMPLATE_LIB_INFO_HPP

#include <string>

namespace fmu4cpp {

    /// Software version
    struct version final {
        int major = 0;
        int minor = 0;
        int patch = 0;
    };

    /// Returns the version of fmu4cpp.
    version library_version();

    // Get a string representation of the version object
    std::string to_string(version v);

}// namespace fmu4cpp

#endif//FMU4CPP_TEMPLATE_LIB_INFO_HPP
