
#ifndef FMU4CPP_TEMPLATE_LOGGER_HPP
#define FMU4CPP_TEMPLATE_LOGGER_HPP

#include <memory>

#include "fmi2/fmi2FunctionTypes.h"

namespace fmu4cpp {
    class logger {

    public:
        logger(fmi2ComponentEnvironment c, fmi2CallbackFunctions f, std::string instanceName)
            : c_(c),
              fmiLogger_(f.logger),
              instanceName_(std::move(instanceName)) {}

        void setDebugLogging(bool flag) {
            debugLogging_ = flag;
        }

        // Logs a message.
        template<typename... Args>
        void log(fmi2Status s, const std::string &message, Args &&...args) {
            if (debugLogging_) {
                msgBuf_ = message;
                fmiLogger_(c_, instanceName_.c_str(), s, msgBuf_.c_str(), "", std::forward<Args>(args)...);
            }
        }

        // Logs a message.
        template<typename... Args>
        void debug(fmi2Status s, const std::string &message, Args &&...args) {
            if (debugLogging_) {
                log(s, message, std::forward<Args>(args)...);
            }
        }

    private:
        bool debugLogging_;
        std::string instanceName_;
        fmi2ComponentEnvironment c_;
        fmi2CallbackLogger fmiLogger_;
        std::string msgBuf_;
    };

}// namespace fmu4cpp

#endif//FMU4CPP_TEMPLATE_LOGGER_HPP
