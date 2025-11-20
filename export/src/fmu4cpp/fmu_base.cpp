
#include "fmu4cpp/fmu_base.hpp"
#include "fmu4cpp/fmu_except.hpp"
#include "fmu4cpp/lib_info.hpp"
#include "fmu4cpp/model_info.hpp"
#include "fmu4cpp/util.hpp"

#include "hash.hpp"

#include <algorithm>
#include <functional>
#include <sstream>
#include <utility>


namespace fmu4cpp {

    void fmu_base::enter_initialisation_mode() {
    }

    void fmu_base::exit_initialisation_mode() {
    }

    void fmu_base::terminate() {
    }

    void fmu_base::reset() {
        throw fatal_error("Reset is unimplemented in slave");
    }

    IntVariable& fmu_base::register_integer(const std::string &name, int *ptr, const std::function<void()> &onChange) {
        const auto vr = static_cast<unsigned int>(numVariables_++);
        auto& v = integers_.emplace_back(name, vr, numVariables_, ptr, onChange);
        vrToIntegerIndices_.emplace(v.value_reference(), integers_.size() - 1);
        return v;
    }

    IntVariable& fmu_base::register_integer(const std::string &name, const std::function<int()> &getter, const std::optional<std::function<void(int)>> &setter) {
        const auto vr = static_cast<unsigned int>(numVariables_++);
        auto& v = integers_.emplace_back(name, vr, numVariables_, getter, setter);
        vrToIntegerIndices_.emplace(v.value_reference(), integers_.size() - 1);
        return v;
    }

    RealVariable& fmu_base::register_real(const std::string &name, double *ptr, const std::function<void()> &onChange) {
        const auto vr = static_cast<unsigned int>(numVariables_++);
        auto& v = reals_.emplace_back(name, vr, numVariables_, ptr, onChange);
        vrToRealIndices_.emplace(v.value_reference(), reals_.size() - 1);
        return v;
    }

    RealVariable& fmu_base::register_real(const std::string &name, const std::function<double()> &getter, const std::optional<std::function<void(double)>> &setter) {
        const auto vr = static_cast<unsigned int>(numVariables_++);
        auto& v = reals_.emplace_back(name, vr, numVariables_, getter, setter);
        vrToRealIndices_.emplace(v.value_reference(), reals_.size() - 1);
        return v;
    }

    BoolVariable& fmu_base::register_boolean(const std::string &name, bool *ptr, const std::function<void()> &onChange) {
        const auto vr = static_cast<unsigned int>(numVariables_++);
        auto& v = booleans_.emplace_back(name, vr, numVariables_, ptr, onChange);
        vrToBooleanIndices_.emplace(v.value_reference(), booleans_.size() - 1);
        return v;
    }

    BoolVariable& fmu_base::register_boolean(const std::string &name, const std::function<bool()> &getter, const std::optional<std::function<void(bool)>> &setter) {
        const auto vr = static_cast<unsigned int>(numVariables_++);
        auto& v = booleans_.emplace_back(name, vr, numVariables_, getter, setter);
        vrToBooleanIndices_.emplace(v.value_reference(), booleans_.size() - 1);
        return v;
    }

    StringVariable& fmu_base::register_string(const std::string &name, std::string *ptr, const std::function<void()> &onChange) {
        const auto vr = static_cast<unsigned int>(numVariables_++);
        auto& v = strings_.emplace_back(name, vr, numVariables_, ptr, onChange);
        vrToStringIndices_.emplace(v.value_reference(), strings_.size() - 1);
        return v;
    }

    StringVariable& fmu_base::register_string(const std::string &name, const std::function<std::string()> &getter, const std::optional<std::function<void(std::string)>> &setter) {
        const auto vr = static_cast<unsigned int>(numVariables_++);
        auto& v = strings_.emplace_back(name, vr, numVariables_, getter, setter);
        vrToStringIndices_.emplace(v.value_reference(), strings_.size() - 1);
        return v;
    }

    BinaryVariable& fmu_base::register_binary(const std::string &name, BinaryType *ptr, const std::function<void()> &onChange) {
        const auto vr = static_cast<unsigned int>(numVariables_++);
        auto& v = binary_.emplace_back(name, vr, numVariables_, ptr, onChange);
        vrToBinaryIndices_.emplace(v.value_reference(), binary_.size() - 1);
        return v;
    }

    BinaryVariable& fmu_base::register_binary(const std::string &name, const std::function<BinaryType()> &getter, const std::optional<std::function<void(BinaryType)>> &setter) {
        const auto vr = static_cast<unsigned int>(numVariables_++);
        auto& v = binary_.emplace_back(name, vr, numVariables_, getter, setter);
        vrToBinaryIndices_.emplace(v.value_reference(), binary_.size() - 1);
        return v;
    }

    [[maybe_unused]] std::string fmu_base::guid() const {
        const model_info info = get_model_info();
        const std::vector content{
                info.author,
                info.version,
                info.description,
                info.modelName,
                model_identifier()};

        std::stringstream ss;
        for (const auto &str: content) {
            ss << str;
        }

        const auto vars = collect(integers_, reals_, booleans_, strings_);
        for (const auto &v: vars) {
            ss << v->name();
            ss << std::to_string(v->index());
            ss << std::to_string(v->value_reference());
            ss << to_string(v->causality());
            if (v->variability()) {
                ss << to_string(*v->variability());
            }
            if (v->initial()) {
                ss << to_string(*v->initial());
            }
        }

        return std::to_string(fnv1a(ss.str()));
    }

    std::vector<unsigned> fmu_base::get_value_refs() const {
        std::vector<unsigned int> indices;
        auto allVars = collect(integers_, reals_, booleans_, strings_);
        indices.reserve(allVars.size());
        for (auto v: allVars) {
            indices.emplace_back(v->value_reference());
        }

        return indices;
    }


}// namespace fmu4cpp
