
#include "fmu4cpp/fmu_base.hpp"
#include "fmu4cpp/lib_info.hpp"
#include "fmu4cpp/time.hpp"
#include "fmu4cpp/util.hpp"

#include <sstream>

using namespace fmu4cpp;


namespace {

    std::string type(const VariableBase *v) {
        if (dynamic_cast<const IntVariable *>(v)) {
            return "Int32";
        } else if (dynamic_cast<const RealVariable *>(v)) {
            return "Float64";
        } else if (dynamic_cast<const StringVariable *>(v)) {
            return "String";
        } else if (dynamic_cast<const BoolVariable *>(v)) {
            return "Boolean";
        }
        throw std::runtime_error("Unknown variable type");
    }

}


std::string fmu_base::make_description() const {

    const model_info m = get_model_info();
    std::stringstream ss;
    ss << R"(<?xml version="1.0" encoding="UTF-8"?>)"
       << "\n"
       << R"(<fmiModelDescription fmiVersion="3.0")"
       << " modelName=\"" << m.modelName << "\""
       << " instantiationToken=\"" << guid() << "\""
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

    // if (!m.vendorAnnotations.empty()) {
    //     ss << "\t<Annotations>\n";
    //     for (const auto &annotation: m.vendorAnnotations) {
    //         std::string indentedAnnotation = indent_multiline_string(annotation, 3);
    //         ss << indentedAnnotation << "\n";
    //     }
    //     ss << "\t</Annotations>\n";
    // }

    ss << "\t<ModelVariables>\n";

    const auto allVars = [&] {
        auto allVars = collect(integers_, reals_, booleans_, strings_);
        std::sort(allVars.begin(), allVars.end(), [](const VariableBase *v1, const VariableBase *v2) {
            return v1->index() < v2->index();
        });
        return allVars;
    }();

    for (const auto &v: allVars) {
        const auto variability = v->variability();
        const auto initial = v->initial();
        const auto annotations = v->getAnnotations();
        ss << "\t\t<!--"
           << "index=" << v->index() << "-->\n"
           << "\t\t<" << type(v) << " name=\""
           << v->name() << "\" valueReference=\"" << v->value_reference() << "\""
           << " causality=\"" << to_string(v->causality()) << "\"";
        if (variability) {
            ss << " variability=\"" << to_string(*variability) << "\"";
        }
        if (initial) {
            ss << " initial=\"" << to_string(*initial) << "\"";
        }
        bool with_start = requires_start(*v);
        if (auto i = dynamic_cast<const IntVariable *>(v)) {
            if (with_start) {
                ss << " start=\"" << i->get() << "\"";
            }
        } else if (auto r = dynamic_cast<const RealVariable *>(v)) {
            if (with_start) {
                ss << " start=\"" << r->get() << "\"";
            }
            const auto min = r->getMin();
            const auto max = r->getMax();
            if (min && max) {
                ss << " min=\"" << *min << "\" max=\"" << *max << "\"";
            }
        } else if (auto s = dynamic_cast<const StringVariable *>(v)) {
            if (with_start) {
                ss << ">\n";
                ss << "\t\t\t<Dimension start=\"1\"/>\n";
                ss << "\t\t\t<Start value=\"" << s->get() << "\"/>\n";
            }
        } else if (auto b = dynamic_cast<const BoolVariable *>(v)) {
            if (with_start) {
                ss << " start=\"" << b->get() << "\"";
            }
        }
        if (with_start && dynamic_cast<const StringVariable *>(v)) {
            ss << "\t\t</String>\n";
        } else {
            ss << "/>\n";
        }

        // if (!annotations.empty()) {
        //     ss << "\t\t\t<Annotations>\n";
        //     for (const auto &annotation: annotations) {
        //         std::string indentedAnnotation = indent_multiline_string(annotation, 4);
        //         ss << indentedAnnotation << "\n";
        //     }
        //     ss << "\t\t\t</Annotations>\n";
        // }
    }

    ss << "\t</ModelVariables>\n";

    ss << "\t<ModelStructure>\n";

    const auto unknowns = collect(integers_, reals_, booleans_, strings_, [](auto &v) {
        return v.causality() == causality_t::OUTPUT;
    });

    if (!unknowns.empty()) {
        for (const auto &v: unknowns) {
            ss << "\t\t\t<Output valueReference=\"" << v->value_reference() << "\"";
            if (const auto deps = v->getDependencies(); !deps.empty()) {
                ss << " dependencies=\"";
                for (unsigned i = 0; i < deps.size(); i++) {
                    const auto &depName = deps[i];
                    const auto dep = std::find_if(allVars.begin(), allVars.end(), [depName](const auto &v) {
                        return v->name() == depName;
                    });
                    if (dep == allVars.end()) {
                        throw std::runtime_error("Unknown dependency: " + depName);
                    }
                    ss << (*dep)->index() - 1;// valueRef is index -1
                    if (i != deps.size() - 1) {
                        ss << " ";
                    }
                }
                ss << "\"";
            }
            ss << "/>\n";
        }
    }

    const auto initialUnknowns = collect(integers_, reals_, booleans_, strings_, [](auto &v) {
        return (v.causality() == causality_t::OUTPUT && v.initial() == initial_t::APPROX || v.initial() == initial_t::CALCULATED) || v.causality() == causality_t::CALCULATED_PARAMETER;
    });
    if (!initialUnknowns.empty()) {
        for (const auto &v: initialUnknowns) {
            ss << "\t\t\t<InitialUnknown valueReference=\"" << v->index() - 1 << "\"";
            ss << "/>\n";
        }
    }

    ss << "\t</ModelStructure>\n";

    ss << "</fmiModelDescription>\n";

    return ss.str();
}