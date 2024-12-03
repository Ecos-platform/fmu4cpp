
#ifndef FMU4CPP_FMU_EXCEPT_HPP
#define FMU4CPP_FMU_EXCEPT_HPP

#include <stdexcept>
#include <string>

namespace fmu4cpp {

    class fatal_error final : public std::runtime_error {
    public:
        explicit fatal_error(const std::string &msg) : std::runtime_error(msg) {}
    };


}// namespace fmu4cpp

#endif//FMU4CPP_FMU_EXCEPT_HPP
