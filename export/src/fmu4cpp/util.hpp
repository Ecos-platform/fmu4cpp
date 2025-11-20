
#ifndef FMU4CPP_UTIL_HPP
#define FMU4CPP_UTIL_HPP

#include <functional>
#include <string>
#include <vector>

#include "fmu4cpp/fmu_base.hpp"


namespace fmu4cpp {

    inline std::vector<const VariableBase *> collect(
            const std::vector<IntVariable> &v1,
            const std::vector<RealVariable> &v2,
            const std::vector<BoolVariable> &v3,
            const std::vector<StringVariable> &v4,
            const std::vector<BinaryVariable> &v5,
            const std::function<bool(const VariableBase &)> &predicate = [](auto &v) { return true; }) {

        std::vector<const VariableBase *> vars;

        const auto add_if_predicate = [&vars, &predicate](const auto &vec) {
            for (const auto &v: vec) {
                if (predicate(v)) {
                    vars.push_back(&v);
                }
            }
        };

        add_if_predicate(v1);
        add_if_predicate(v2);
        add_if_predicate(v3);
        add_if_predicate(v4);
        add_if_predicate(v5);

        return vars;
    }

    inline std::vector<const VariableBase *> collect(
            const std::vector<IntVariable> &v1,
            const std::vector<RealVariable> &v2,
            const std::vector<BoolVariable> &v3,
            const std::vector<StringVariable> &v4,
            const std::function<bool(const VariableBase &)> &predicate = [](auto &v) { return true; }) {

        return collect(v1, v2, v3, v4, {}, predicate);
    }


    inline std::string indent_multiline_string(const std::string &input, const int indents) {
        const std::string tabs(indents, '\t');
        std::string indentedString = tabs + input;// add initial indentation
        size_t pos = 0;
        while ((pos = indentedString.find('\n', pos)) != std::string::npos) {
            indentedString.replace(pos, 1, "\n" + tabs);
            pos += 3;
        }
        return indentedString;
    }

}// namespace fmu4cpp

#endif//FMU4CPP_UTIL_HPP
