
#ifndef FMU4CPP_TEMPLATE_LOGGER_HPP
#define FMU4CPP_TEMPLATE_LOGGER_HPP

#include "fmi2/fmi2FunctionTypes.h"

namespace fmu4cpp {

    class logger final {

    public:
        logger(const fmi2CallbackFunctions &f, std::string instanceName)
            : c_(f.componentEnvironment),
              fmiLogger_(f.logger),
              instanceName_(std::move(instanceName)) {}

        void setDebugLogging(bool flag) {
            debugLogging_ = flag;
        }

        // Logs a message.
        template<typename... Args>
        void log(const fmi2Status s, const std::string &message, Args &&...args) {
            if (debugLogging_) {
                fmiLogger_(c_, instanceName_.c_str(), s, nullptr, message.c_str(), std::forward<Args>(args)...);
            }
        }

    private:
        fmi2ComponentEnvironment c_;
        fmi2CallbackLogger fmiLogger_;

        bool debugLogging_{false};
        std::string instanceName_;
    };

}// namespace fmu4cpp

#endif//FMU4CPP_TEMPLATE_LOGGER_HPP
