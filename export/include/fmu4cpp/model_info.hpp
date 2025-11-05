
#ifndef FMU4CPP_TEMPLATE_MODEL_INFO_HPP
#define FMU4CPP_TEMPLATE_MODEL_INFO_HPP

#include <string>
#include <optional>
#include <vector>

namespace fmu4cpp {

    std::string model_identifier();

    struct default_experiment final {
        double startTime{0.0};
        std::optional<double> stopTime;
        std::optional<double> stepSize;
        std::optional<double> tolerance;
    };

    struct model_info final {
        std::string modelName;
        std::string author;
        std::string description;
        std::string version;
        std::string variableNamingConvention{"structured"};
        std::vector<std::string> vendorAnnotations;
        bool needsExecutionTool{false};
        bool canHandleVariableCommunicationStepSize{true};
        bool canBeInstantiatedOnlyOncePerProcess{false};
        bool canGetAndSetFMUstate{false};
        bool canSerializeFMUstate{false};

        std::optional<default_experiment> defaultExperiment;
    };

}// namespace fmu4cpp

#endif//FMU4CPP_TEMPLATE_MODEL_INFO_HPP
