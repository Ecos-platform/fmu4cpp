
#ifndef FMU4CPP_TEMPLATE_LOGGER_HPP
#define FMU4CPP_TEMPLATE_LOGGER_HPP

#include "fmu4cpp/status.hpp"

namespace fmu4cpp {

    class logger {

    public:
        explicit logger(std::string instanceName)
            : instanceName_(std::move(instanceName)) {}

        void setDebugLogging(bool flag) {
            debugLogging_ = flag;
        }

        // Logs a message.
        void log(fmiStatus s, const std::string &message) {
            if (debugLogging_) {
                debugLog(s, message);
            }
        }

        virtual ~logger() = default;

    private:
        bool debugLogging_{false};

    protected:
        std::string instanceName_;

        virtual void debugLog(fmiStatus s, const std::string &message) = 0;
    };

}// namespace fmu4cpp

#endif//FMU4CPP_TEMPLATE_LOGGER_HPP
