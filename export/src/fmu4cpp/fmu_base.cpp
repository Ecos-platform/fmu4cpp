
#include "fmu4cpp/fmu_base.hpp"
#include "fmu4cpp/fmu_except.hpp"
#include "fmu4cpp/lib_info.hpp"
#include "fmu4cpp/model_info.hpp"

#include "hash.hpp"
#include "time.hpp"

#include <algorithm>
#include <functional>
#include <sstream>
#include <utility>
#include <variant>

namespace {

    using VariableRef = std::variant<
            std::reference_wrapper<const fmu4cpp::IntVariable>,
            std::reference_wrapper<const fmu4cpp::RealVariable>,
            std::reference_wrapper<const fmu4cpp::BoolVariable>,
            std::reference_wrapper<const fmu4cpp::StringVariable>>;

    std::vector<VariableRef> collect(
            const std::vector<fmu4cpp::IntVariable> &v1,
            const std::vector<fmu4cpp::RealVariable> &v2,
            const std::vector<fmu4cpp::BoolVariable> &v3,
            const std::vector<fmu4cpp::StringVariable> &v4,
            const std::function<bool(const fmu4cpp::VariableBase &)> &predicate = [](auto &v) { return true; }) {

        std::vector<VariableRef> vars;

        const auto add_if_predicate = [&vars, &predicate](const auto &vec) {
            for (const auto &v: vec) {
                if (predicate(v)) {
                    vars.emplace_back(v);
                }
            }
        };

        add_if_predicate(v1);
        add_if_predicate(v2);
        add_if_predicate(v3);
        add_if_predicate(v4);

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

    std::string indent_multiline_string(const std::string &input, const int indents) {
        const std::string tabs(indents, '\t');
        std::string indentedString = tabs + input;// add initial indentation
        size_t pos = 0;
        while ((pos = indentedString.find('\n', pos)) != std::string::npos) {
            indentedString.replace(pos, 1, "\n" + tabs);
            pos += 3;
        }
        return indentedString;
    }

    std::string fmu_base::make_description() const {

        const model_info m = get_model_info();
        std::stringstream ss;
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

        if (!m.vendorAnnotations.empty()) {
            ss << "\t<VendorAnnotations>\n";
            for (const auto &annotation: m.vendorAnnotations) {
                std::string indentedAnnotation = indent_multiline_string(annotation, 3);
                ss << indentedAnnotation << "\n";
            }
            ss << "\t</VendorAnnotations>\n";
        }

        ss << "\t<ModelVariables>\n";

        const auto allVars = [&] {
            auto allVars = collect(integers_, reals_, booleans_, strings_);
            std::sort(allVars.begin(), allVars.end(), [](const VariableRef &v1, const VariableRef &v2) {
                return std::visit([](const auto &var1Ref, const auto &var2Ref) {
                    return var1Ref.get().index() < var2Ref.get().index();
                },
                                  v1, v2);
            });
            return allVars;
        }();
        for (const auto &var: allVars) {
            std::visit([&ss](const auto &varRef) {
                const auto &v = varRef.get();
                const auto variability = v.variability();
                const auto initial = v.initial();
                const auto annotations = v.getAnnotations();
                ss << "\t\t<!--"
                   << "index=" << v.index() << "-->\n"
                   << "\t\t<ScalarVariable name=\""
                   << v.name() << "\" valueReference=\"" << v.value_reference() << "\""
                   << " causality=\"" << to_string(v.causality()) << "\"";
                if (variability) {
                    ss << " variability=\"" << to_string(*variability) << "\"";
                }
                if (initial) {
                    ss << " initial=\"" << to_string(*initial) << "\"";
                }
                ss << ">\n";
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, IntVariable>) {
                    ss << "\t\t\t<Integer";
                    if (requires_start(v)) {
                        ss << " start=\"" << v.get() << "\"";
                    }
                } else if constexpr (std::is_same_v<T, RealVariable>) {
                    ss << "\t\t\t<Real";
                    if (requires_start(v)) {
                        ss << " start=\"" << v.get() << "\"";
                    }
                    const auto min = v.getMin();
                    const auto max = v.getMax();
                    if (min && max) {
                        ss << " min=\"" << *min << "\" max=\"" << *max << "\"";
                    }
                } else if constexpr (std::is_same_v<T, StringVariable>) {
                    ss << "\t\t\t<String";
                    if (requires_start(v)) {
                        ss << " start=\"" << v.get() << "\"";
                    }
                } else if constexpr (std::is_same_v<T, BoolVariable>) {
                    ss << "\t\t\t<Boolean";
                    if (requires_start(v)) {
                        ss << " start=\"" << v.get() << "\"";
                    }
                }
                ss << "/>\n";
                if (!annotations.empty()) {
                    ss << "\t\t\t<Annotations>\n";
                    for (const auto &annotation: annotations) {
                        const auto indentedAnnotation = indent_multiline_string(annotation, 4);
                        ss << indentedAnnotation << "\n";
                    }
                    ss << "\t\t\t</Annotations>\n";
                }
                ss << "\t\t</ScalarVariable>"
                   << "\n";
            },
                       var);
        }

        ss << "\t</ModelVariables>\n";

        ss << "\t<ModelStructure>\n";

        if (const auto unknowns = collect(integers_, reals_, booleans_, strings_, [](auto &v) {
                return v.causality() == causality_t::OUTPUT;
            });
            !unknowns.empty()) {
            ss << "\t\t<Outputs>\n";
            for (const auto &var: unknowns) {
                std::visit([&ss](const auto &varRef) {
                    const auto &v = varRef.get();
                    ss << "\t\t\t<Unknown index=\"" << v.index() << "\"";
                    const auto deps = v.getDependencies();
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
                },
                           var);
            }
            ss << "\t\t</Outputs>\n";
        }

        if (const auto initialUnknowns = collect(integers_, reals_, booleans_, strings_, [](auto &v) {
                return (v.causality() == causality_t::OUTPUT && v.initial() == initial_t::APPROX || v.initial() == initial_t::CALCULATED) || v.causality() == causality_t::CALCULATED_PARAMETER;
            });
            !initialUnknowns.empty()) {
            ss << "\t\t<InitialUnknowns>\n";
            for (const auto &var: initialUnknowns) {
                std::visit([&ss](const auto &varRef) {
                    const auto &v = varRef.get();
                    ss << "\t\t\t<Unknown index=\"" << v.index() << "\"";
                    ss << "/>\n";
                },
                           var);
            }
            ss << "\t\t</InitialUnknowns>\n";
        }

        ss << "\t</ModelStructure>\n";

        ss << "</fmiModelDescription>\n";

        return ss.str();
    }

    IntVariable fmu_base::integer(const std::string &name, int *ptr) {
        return {name, static_cast<unsigned int>(integers_.size()), numVariables_++, ptr};
    }

    IntVariable fmu_base::integer(const std::string &name, const std::function<int()> &getter, const std::optional<std::function<void(int)>> &setter) {
        return {name, static_cast<unsigned int>(integers_.size()), numVariables_++, getter, setter};
    }

    RealVariable fmu_base::real(const std::string &name, double *ptr) {
        return {name, static_cast<unsigned int>(reals_.size()), numVariables_++, ptr};
    }

    RealVariable fmu_base::real(const std::string &name, const std::function<double()> &getter, const std::optional<std::function<void(double)>> &setter) {
        return {name, static_cast<unsigned int>(reals_.size()), numVariables_++, getter, setter};
    }

    BoolVariable fmu_base::boolean(const std::string &name, bool *ptr) {
        return {name, static_cast<unsigned int>(booleans_.size()), numVariables_++, ptr};
    }

    BoolVariable fmu_base::boolean(const std::string &name, const std::function<bool()> &getter, const std::optional<std::function<void(bool)>> &setter) {
        return {name, static_cast<unsigned int>(booleans_.size()), numVariables_++, getter, setter};
    }

    StringVariable fmu_base::string(const std::string &name, std::string *ptr) {
        return {name, static_cast<unsigned int>(strings_.size()), numVariables_++, ptr};
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
        const model_info info = get_model_info();
        const std::vector content{
                info.author,
                info.version,
                info.modelIdentifier,
                info.description,
                info.modelName};

        std::stringstream ss;
        for (const auto &str: content) {
            ss << str;
        }

        const auto vars = collect(integers_, reals_, booleans_, strings_);
        for (const auto &var: vars) {
            std::visit([&ss](const auto &varRef) {
                const auto &v = varRef.get();
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
            },
                       var);
        }

        return std::to_string(fnv1a(ss.str()));
    }

}// namespace fmu4cpp
