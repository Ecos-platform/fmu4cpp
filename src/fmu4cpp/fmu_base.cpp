
#include "fmu4cpp/fmu_base.hpp"

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

    void fmu_base::register_int(const std::string &name, const std::function<int()> &getter, const std::optional<std::function<void(int)>> &setter) {
        IntVariable v(name, integers_.size(), getter, setter);
        integers_.emplace_back(std::move(v));
    }
    void fmu_base::register_real(const std::string &name, const std::function<double()> &getter, const std::optional<std::function<void(double)>> &setter) {
        RealVariable v(name, reals_.size(), getter, setter);
        reals_.emplace_back(std::move(v));
    }

    void fmu_base::register_bool(const std::string &name, const std::function<bool()> &getter, const std::optional<std::function<void(bool)>> &setter) {
        BoolVariable v(name, booleans_.size(), getter, setter);
        booleans_.emplace_back(std::move(v));
    }

    void fmu_base::register_string(const std::string &name, const std::function<std::string()> &getter, const std::optional<std::function<void(std::string)>> &setter) {
        StringVariable v(name, strings_.size(), getter, setter);
        strings_.emplace_back(std::move(v));
    }

    std::string fmu_base::make_description() const {

        std::stringstream ss;

        ss << R"(<?xml version="1.0" encoding="UTF-8"?>)"
           << "\n"
           << R"(<fmiModelDescription fmiVersion="2.0")"
           << " modelName=\"" << modelName() << "\""
           << " guid=\"" << uuid::generate_uuid_v4() << "\""
           << R"( generationTool="fmu4cpp")"
           << " description=\"" << description() << "\""
           << " author=\"" << author() << "\""
           << ">\n";

        ss << "\t"
           << R"(<CoSimulation needsExecutionTool="false")"
           << " modelIdentifier=\"" << modelName() << "\""
           << R"( canHandleVariableCommunicationStepSize="true")"
           << R"( canBeInstantiatedOnlyOncePerProcess="false")"
           << R"( canGetAndSetFMUstate="false")"
           << R"( canSerializeFMUstate="false")"
           << R"( canNotUseMemoryManagementFunctions="true")"
           << ">\n"
           << "\t</CoSimulation>"
           << "\n";

        ss << "\t<ModelVariables>\n";

        for (const auto &v: integers_) {
            ss << "\t\t<ScalarVariable name=\""
               << v.name() << "\" valueReference=\"" << v.value_reference() << "\">";
            ss << "\n\t\t\t<Integer start=\"" << v.get() << "\"/>"
               << "\n";
            ss << "\t\t</ScalarVariable>"
               << "\n";
        }

        for (const auto &v: reals_) {
            ss << "\t\t<ScalarVariable name=\""
               << v.name() << "\" valueReference=\"" << v.value_reference() << "\">";
            ss << "\n\t\t\t<Real start=\"" << v.get() << "\"/>"
               << "\n";
            ss << "\t\t</ScalarVariable>"
               << "\n";
        }

        for (const auto &v: booleans_) {
            ss << "\t\t<ScalarVariable name=\""
               << v.name() << "\" valueReference=\"" << v.value_reference() << "\">";
            ss << "\n\t\t\t<Boolean start=\"" << v.get() << "\"/>"
               << "\n";
            ss << "\t\t</ScalarVariable>"
               << "\n";
        }

        for (const auto &v: strings_) {
            ss << "\t\t<ScalarVariable name=\""
               << v.name() << "\" valueReference=\"" << v.value_reference() << "\">";
            ss << "\n\t\t\t<String start=\"" << v.get() << "\"/>"
               << "\n";
            ss << "\t\t</ScalarVariable>"
               << "\n";
        }


        ss << "\t</ModelVariables>\n";

        ss << "\t<ModelStructure>\n";


        std::vector<int> outputs;
        int index = 0;
        for (const auto &i: integers_) {
            if (i.causality() == causality_t::OUTPUT) {
                outputs.push_back(index);
            }
            index++;
        }
        for (const auto &i: reals_) {
            if (i.causality() == causality_t::OUTPUT) {
                outputs.push_back(index);
            }
            index++;
        }
        for (const auto &i: booleans_) {
            if (i.causality() == causality_t::OUTPUT) {
                outputs.push_back(index);
            }
            index++;
        }
        for (const auto &i: strings_) {
            if (i.causality() == causality_t::OUTPUT) {
                outputs.push_back(index);
            }
            index++;
        }

        if (!outputs.empty()) {
            ss << "\t\t<Outputs>\n";
            for (auto i: outputs) {
                ss << "\t\t\tUnknown index=\"" << i << "\"\n";
            }
            ss << "\t\t</Outputs>\n";
        }

        ss << "\t</ModelStructure>\n";

        ss << "</fmiModelDescription";


        return ss.str();
    }


}// namespace fmu4cpp
