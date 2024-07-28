
#include "fmu4cpp/fmu_base.hpp"
#include "fmu4cpp/lib_info.hpp"
#include "fmu4cpp/model_info.hpp"

#include "hash.hpp"
#include "time.hpp"
#include <functional>
#include <sstream>
#include <utility>

namespace {

    std::vector<fmu4cpp::VariableBase> collect(
            const std::vector<fmu4cpp::IntVariable> &v1,
            const std::vector<fmu4cpp::RealVariable> &v2,
            const std::vector<fmu4cpp::BoolVariable> &v3,
            const std::vector<fmu4cpp::StringVariable> &v4,
            const std::function<bool(const fmu4cpp::VariableBase &)> &f = [](auto &v) { return true; }) {
        std::vector<fmu4cpp::VariableBase> vars;
        for (const fmu4cpp::VariableBase &v: v1) {
            if (f(v)) {
                vars.push_back(v);
            }
        }
        for (const fmu4cpp::VariableBase &v: v2) {
            if (f(v)) {
                vars.push_back(v);
            }
        }
        for (const fmu4cpp::VariableBase &v: v3) {
            if (f(v)) {
                vars.push_back(v);
            }
        }
        for (const fmu4cpp::VariableBase &v: v4) {
            if (f(v)) {
                vars.push_back(v);
            }
        }
        return vars;
    }
}// namespace

namespace fmu4cpp {

    void fmu_base::setup_experiment(double start, std::optional<double> stop, std::optional<double> tolerance) {
    }

    void fmu_base::enter_initialisation_mode() {
    }

    void fmu_base::exit_initialisation_mode() {
    }

    void fmu_base::terminate() {
    }

    void fmu_base::reset() {
        throw fatal_error("Reset is unimplemented in slave");
    }

    std::string fmu_base::make_description() const {

        std::stringstream ss;
        model_info m = get_model_info();

        ss << R"(<?xml version="1.0" encoding="UTF-8"?>)"
           << "\n"
           << R"(<fmiModelDescription fmiVersion="2.0")"
           << " modelName=\"" << m.modelName << "\""
           << " guid=\"" << guid() << "\""
           << " generationTool=\"fmu4cpp"
           << " v" << to_string(library_version()) << "\""
           << " generationDateAndTime=\"" << now() << "\""
           << " description=\"" << m.description << "\""
           << " author=\"" << m.author << "\""
           << " variableNamingConvention=\"" << m.variableNamingConvention << "\""
           << ">\n";

        ss << "\t" << std::boolalpha
           << "<CoSimulation needsExecutionTool=\"" << m.needsExecutionTool << "\""
           << " modelIdentifier=\"" << m.modelIdentifier << "\""
           << " canHandleVariableCommunicationStepSize=\"" << m.canHandleVariableCommunicationStepSize << "\""
           << " canBeInstantiatedOnlyOncePerProcess=\"" << m.canBeInstantiatedOnlyOncePerProcess << "\""
           << " canGetAndSetFMUstate=\"" << m.canGetAndSetFMUstate << "\""
           << " canSerializeFMUstate=\"" << m.canSerializeFMUstate << "\""
           << R"( canNotUseMemoryManagementFunctions="true")"
           << ">\n"
           << "\t</CoSimulation>"
           << "\n";

        ss << "\t<ModelVariables>\n";

        for (const auto &v: integers_) {
            auto variability = v.variability();
            auto initial = v.initial();
            ss << "\t\t<ScalarVariable name=\""
               << v.name() << "\" valueReference=\"" << v.value_reference() << "\""
               << " causality=\"" << to_string(v.causality()) << "\"";
            if (variability) {
                ss << " variability=\"" << to_string(*variability) << "\"";
            }
            if (initial) {
                ss << " initial=\"" << to_string(*initial) << "\"";
            }
            ss << ">\n";
            ss << "\t\t\t<Integer";
            if (requires_start(v)) {
                ss << " start=\"" << v.get() << "\"";
            }
            ss << "/>\n";
            ss << "\t\t</ScalarVariable>"
               << "\n";
        }

        for (const auto &v: reals_) {
            auto variability = v.variability();
            auto initial = v.initial();
            ss << "\t\t<ScalarVariable name=\""
               << v.name() << "\" valueReference=\"" << v.value_reference() << "\""
               << " causality=\"" << to_string(v.causality()) << "\"";
            if (variability) {
                ss << " variability=\"" << to_string(*variability) << "\"";
            }
            if (initial) {
                ss << " initial=\"" << to_string(*initial) << "\"";
            }
            ss << ">\n";
            ss << "\t\t\t<Real";
            if (requires_start(v)) {
                ss << " start=\"" << v.get() << "\"";
            }
            auto min = v.getMin();
            auto max = v.getMax();
            if (min && max) {
                ss << " min=\"" << *min << "\" max=\"" << *max << "\"";
            }
            ss << "/>\n";
            ss << "\t\t</ScalarVariable>"
               << "\n";
        }

        for (const auto &v: booleans_) {
            auto variability = v.variability();
            auto initial = v.initial();
            ss << "\t\t<ScalarVariable name=\""
               << v.name() << "\" valueReference=\"" << v.value_reference() << "\""
               << " causality=\"" << to_string(v.causality()) << "\"";
            if (variability) {
                ss << " variability=\"" << to_string(*variability) << "\"";
            }
            if (initial) {
                ss << " initial=\"" << to_string(*initial) << "\"";
            }
            ss << ">\n";
            ss << "\t\t\t<Boolean";
            if (requires_start(v)) {
                ss << " start=\"" << v.get() << "\"";
            }
            ss << "/>\n";
            ss << "\t\t</ScalarVariable>"
               << "\n";
        }

        for (const auto &v: strings_) {
            auto variability = v.variability();
            auto initial = v.initial();
            ss << "\t\t<ScalarVariable name=\""
               << v.name() << "\" valueReference=\"" << v.value_reference() << "\""
               << " causality=\"" << to_string(v.causality()) << "\"";
            if (variability) {
                ss << " variability=\"" << to_string(*variability) << "\"";
            }
            if (initial) {
                ss << " initial=\"" << to_string(*initial) << "\"";
            }
            ss << ">\n";
            ss << "\t\t\t<String";
            if (requires_start(v)) {
                ss << " start=\"" << v.get() << "\"";
            }
            ss << "/>\n";
            ss << "\t\t</ScalarVariable>"
               << "\n";
        }

        ss << "\t</ModelVariables>\n";

        ss << "\t<ModelStructure>\n";

        std::vector<VariableBase> outputs = collect(integers_, reals_, booleans_, strings_, [](auto &v) {
            return v.causality() == causality_t::OUTPUT;
        });

        if (!outputs.empty()) {
            ss << "\t\t<Outputs>\n";
            for (const auto &v: outputs) {
                ss << "\t\t\t<Unknown index=\"" << v.index() << "\"";
                auto deps = v.getDependencies();
                if (!deps.empty()) {
                    ss << " dependencies=\"";
                    for (unsigned i = 0; i < deps.size(); i++) {
                        ss << deps[i];
                        if (i != deps.size() - 1) {
                            ss << " ";
                        }
                    }
                    ss << "\"";
                }
                ss << "/>\n";
            }
            ss << "\t\t</Outputs>\n";
        }

        ss << "\t</ModelStructure>\n";

        ss << "</fmiModelDescription>";

        return ss.str();
    }

    IntVariable fmu_base::integer(const std::string &name, const std::function<int()> &getter, const std::optional<std::function<void(int)>> &setter) {
        return {name, static_cast<unsigned int>(integers_.size()), numVariables_++, getter, setter};
    }

    RealVariable fmu_base::real(const std::string &name, const std::function<double()> &getter, const std::optional<std::function<void(double)>> &setter) {
        return {name, static_cast<unsigned int>(reals_.size()), numVariables_++, getter, setter};
    }

    BoolVariable fmu_base::boolean(const std::string &name, const std::function<bool()> &getter, const std::optional<std::function<void(bool)>> &setter) {
        return {name, static_cast<unsigned int>(booleans_.size()), numVariables_++, getter, setter};
    }

    StringVariable fmu_base::string(const std::string &name, const std::function<std::string()> &getter, const std::optional<std::function<void(std::string)>> &setter) {
        return {name, static_cast<unsigned int>(strings_.size()), numVariables_++, getter, setter};
    }

    void fmu_base::register_variable(IntVariable v) {
        integers_.emplace_back(std::move(v));
    }

    void fmu_base::register_variable(RealVariable v) {
        reals_.emplace_back(std::move(v));
    }

    void fmu_base::register_variable(BoolVariable v) {
        booleans_.emplace_back(std::move(v));
    }

    void fmu_base::register_variable(StringVariable v) {
        strings_.emplace_back(std::move(v));
    }

    [[maybe_unused]] std::string fmu_base::guid() const {
        model_info info = get_model_info();
        std::vector<std::string> content{
                info.author,
                info.version,
                info.modelIdentifier,
                info.description,
                info.modelName};

        std::stringstream ss;
        for (const auto &str: content) {
            ss << str;
        }

        auto vars = collect(integers_, reals_, booleans_, strings_);
        for (const auto &v: vars) {
            ss << v.name();
            ss << std::to_string(v.index());
            ss << std::to_string(v.value_reference());
            ss << to_string(v.causality());
            if (v.variability()) {
                ss << to_string(*v.variability());
            }
            if (v.initial()) {
                ss << to_string(*v.initial());
            }
        }

        return std::to_string(fnv1a(ss.str()));
    }

}// namespace fmu4cpp
