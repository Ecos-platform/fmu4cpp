
#ifndef FMU4CPP_TEMPLATE_MODEL_INFO_HPP
#define FMU4CPP_TEMPLATE_MODEL_INFO_HPP

#include <string>

namespace fmu4cpp {

    struct model_info final {
        std::string modelName;
        std::string author;
        std::string description;
        std::string modelIdentifier;
        std::string version;
        std::string variableNamingConvention{"structured"};
        std::vector<std::string> vendorAnnotations;
        bool needsExecutionTool{false};
        bool canHandleVariableCommunicationStepSize{true};
        bool canBeInstantiatedOnlyOncePerProcess{false};
        bool canGetAndSetFMUstate{false};
        bool canSerializeFMUstate{false};
    };

}// namespace fmu4cpp

#endif//FMU4CPP_TEMPLATE_MODEL_INFO_HPP
