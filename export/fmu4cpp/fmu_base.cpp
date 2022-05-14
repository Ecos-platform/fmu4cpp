
#include "fmu_base.hpp"
#include "lib_info.hpp"
#include "model_info.hpp"

#include "time.hpp"
#include "uuid.hpp"
#include <sstream>
#include <utility>

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
           << " guid=\"" << uuid::generate_uuid_v4() << "\""
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

        std::vector<VariableBase> outputs;
        for (const auto &v: integers_) {
            if (v.causality() == causality_t::OUTPUT) outputs.push_back(v);
        }
        for (const auto &v: reals_) {
            if (v.causality() == causality_t::OUTPUT) outputs.push_back(v);
        }
        for (const auto &v: booleans_) {
            if (v.causality() == causality_t::OUTPUT) outputs.push_back(v);
        }
        for (const auto &v: strings_) {
            if (v.causality() == causality_t::OUTPUT) outputs.push_back(v);
        }

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
        return {name, static_cast<unsigned int>(integers_.size()), numVariables++, getter, setter};
    }

    RealVariable fmu_base::real(const std::string &name, const std::function<double()> &getter, const std::optional<std::function<void(double)>> &setter) {
        return {name, static_cast<unsigned int>(reals_.size()), numVariables++, getter, setter};
    }

    BoolVariable fmu_base::boolean(const std::string &name, const std::function<bool()> &getter, const std::optional<std::function<void(bool)>> &setter) {
        return {name, static_cast<unsigned int>(booleans_.size()), numVariables++, getter, setter};
    }

    StringVariable fmu_base::string(const std::string &name, const std::function<std::string()> &getter, const std::optional<std::function<void(std::string)>> &setter) {
        return {name, static_cast<unsigned int>(strings_.size()), numVariables++, getter, setter};
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

}// namespace fmu4cpp
