
#include "fmi2/fmi2Functions.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

#include "fmu4cpp/fmu_base.hpp"
#include "fmu4cpp/fmu_except.hpp"
#include "fmu4cpp/logger.hpp"

namespace {

    fmiStatus toCommonStatusFromFmi2(fmi2Status status) {
        switch (status) {
            case fmi2OK:
                return fmiOK;
            case fmi2Warning:
                return fmiWarning;
            case fmi2Discard:
                return fmiDiscard;
            case fmi2Error:
                return fmiError;
            case fmi2Fatal:
                return fmiFatal;
            case fmi2Pending:
                return fmiPending;
            default:
                return fmiStatusUnknown;
        }
    }

    fmi2Status toFmi2StatusFromCommon(fmiStatus status) {
        switch (status) {
            case fmiOK:
                return fmi2OK;
            case fmiWarning:
                return fmi2Warning;
            case fmiDiscard:
                return fmi2Discard;
            case fmiError:
                return fmi2Error;
            case fmiFatal:
                return fmi2Fatal;
            case fmiPending:
                return fmi2Pending;
            default:
                return fmi2Error;
        }
    }

    class fmi2Logger : public fmu4cpp::logger {

    public:
        explicit fmi2Logger(const std::string &instanceName, const fmi2CallbackFunctions* f)
            : logger(instanceName), f_(f) {}

    protected:
        void debugLog(fmiStatus s, const std::string &message) override {
            f_->logger(f_->componentEnvironment, instanceName_.c_str(),
                      toFmi2StatusFromCommon(s), nullptr, message.c_str());
        }

    private:
        const fmi2CallbackFunctions* f_;
    };

    // A struct that holds all the data for one model instance.
    struct Component {

        Component(std::unique_ptr<fmu4cpp::fmu_base> slave, const fmi2CallbackFunctions* callbackFunctions)
            : lastSuccessfulTime{std::numeric_limits<double>::quiet_NaN()},
              slave(std::move(slave)),
              logger(std::make_unique<fmi2Logger>(this->slave->instanceName(), callbackFunctions)) {
            this->slave->__set_logger(logger.get());
        }

        double lastSuccessfulTime;
        std::unique_ptr<fmu4cpp::fmu_base> slave;
        std::unique_ptr<fmu4cpp::logger> logger;
    };

}// namespace

extern "C" {

const char *fmi2GetTypesPlatform() {
    return fmi2TypesPlatform;
}

const char *fmi2GetVersion(void) {
    return "2.0";
}

FMI2_Export void write_description(const char *location) {
    const auto instance = fmu4cpp::createInstance("", "");
    const auto xml = instance->make_description_v2();
    std::ofstream of(location);
    of << xml;
    of.close();
}

fmi2Component fmi2Instantiate(fmi2String instanceName,
                              fmi2Type fmuType,
                              fmi2String fmuGUID,
                              fmi2String fmuResourceLocation,
                              const fmi2CallbackFunctions *functions,
                              fmi2Boolean /*visible*/,
                              fmi2Boolean loggingOn) {

    if (fmuType != fmi2CoSimulation) {
        std::cerr << "[fmu4cpp] Error. Unsupported fmuType!" << std::endl;
        return nullptr;
    }

    int magic = 1;
#ifdef _MSC_VER
    magic = 0;
#endif

    std::string resources(fmuResourceLocation);

    if (resources.find("file:////") != std::string::npos) {
        resources.replace(0, 9 - magic, "");
    } else if (resources.find("file:///") != std::string::npos) {
        resources.replace(0, 8 - magic, "");
    } else if (resources.find("file://") != std::string::npos) {
        resources.replace(0, 7 - magic, "");
    } else if (resources.find("file:/") != std::string::npos) {
        resources.replace(0, 6 - magic, "");
    }

    auto slave = fmu4cpp::createInstance(instanceName, resources);
    const auto guid = slave->guid();
    if (guid != fmuGUID) {
        std::cerr << "[fmu4cpp] Error. Wrong guid!" << std::endl;
        fmi2Logger l(instanceName, functions);
        l.log(fmiFatal, "Error. Wrong guid!");
        return nullptr;
    }

    auto c = std::make_unique<Component>(std::move(slave), functions);
    c->logger->setDebugLogging(loggingOn);
    return c.release();
}

fmi2Status fmi2SetupExperiment(fmi2Component c,
                               fmi2Boolean toleranceDefined,
                               fmi2Real tolerance,
                               fmi2Real startTime,
                               fmi2Boolean stopTimeDefined,
                               fmi2Real stopTime) {
    std::optional<double> stop;
    std::optional<double> tol;

    if (stopTimeDefined) stop = stopTime;
    if (toleranceDefined) tol = tolerance;

    const auto component = static_cast<Component *>(c);
    try {
        component->slave->setup_experiment(startTime, stop, tol);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2EnterInitializationMode(fmi2Component c) {
    const auto component = static_cast<Component *>(c);

    try {
        component->slave->enter_initialisation_mode();
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2ExitInitializationMode(fmi2Component c) {
    const auto component = static_cast<Component *>(c);
    try {
        component->slave->exit_initialisation_mode();
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2Terminate(fmi2Component c) {
    const auto component = static_cast<Component *>(c);
    try {
        component->slave->terminate();
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2DoStep(
        fmi2Component c,
        fmi2Real currentCommunicationPoint,
        fmi2Real communicationStepSize,
        fmi2Boolean /*noSetFMUStatePriorToCurrentPoint*/) {

    const auto component = static_cast<Component *>(c);
    try {
        if (component->slave->do_step(currentCommunicationPoint, communicationStepSize)) {
            component->lastSuccessfulTime = currentCommunicationPoint + communicationStepSize;
            return fmi2OK;
        }

        return fmi2Discard;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2CancelStep(fmi2Component) {
    return fmi2Error;
}

fmi2Status fmi2Reset(fmi2Component c) {
    const auto component = static_cast<Component *>(c);
    component->slave->reset();
    return fmi2OK;
}

fmi2Status fmi2GetInteger(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        fmi2Integer value[]) {

    const auto component = static_cast<Component *>(c);
    try {
        component->slave->get_integer(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2GetReal(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        fmi2Real value[]) {

    const auto component = static_cast<Component *>(c);
    try {
        component->slave->get_real(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2GetBoolean(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        fmi2Boolean value[]) {

    const auto component = static_cast<Component *>(c);
    try {
        component->slave->get_boolean(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2GetString(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        fmi2String value[]) {

    const auto component = static_cast<Component *>(c);
    try {
        component->slave->get_string(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2SetInteger(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        const fmi2Integer value[]) {

    const auto component = static_cast<Component *>(c);
    try {
        component->slave->set_integer(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2SetReal(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        const fmi2Real value[]) {

    const auto component = static_cast<Component *>(c);
    try {
        component->slave->set_real(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2SetBoolean(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        const fmi2Boolean value[]) {

    const auto component = static_cast<Component *>(c);
    try {
        component->slave->set_boolean(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2SetString(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        const fmi2String value[]) {

    const auto component = static_cast<Component *>(c);
    try {
        component->slave->set_string(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Fatal), ex.what());
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi2(fmi2Error), ex.what());
        return fmi2Error;
    }
}

fmi2Status fmi2GetStatus(
        fmi2Component,
        const fmi2StatusKind,
        fmi2Status *) {

    return fmi2Error;
}

fmi2Status fmi2GetRealStatus(
        fmi2Component c,
        const fmi2StatusKind s,
        fmi2Real *value) {

    const auto component = static_cast<Component *>(c);
    if (s == fmi2LastSuccessfulTime) {
        *value = component->lastSuccessfulTime;
        return fmi2OK;
    }

    return fmi2Error;
}

fmi2Status fmi2GetIntegerStatus(
        fmi2Component,
        const fmi2StatusKind,
        fmi2Integer *) {
    return fmi2Error;
}

fmi2Status fmi2GetBooleanStatus(
        fmi2Component,
        const fmi2StatusKind,
        fmi2Boolean *) {
    return fmi2Error;
}

fmi2Status fmi2GetStringStatus(
        fmi2Component,
        const fmi2StatusKind,
        fmi2String *) {

    return fmi2Error;
}


fmi2Status fmi2SetDebugLogging(fmi2Component c,
                               fmi2Boolean loggingOn,
                               size_t /*nCategories*/,
                               const fmi2String /*categories*/[]) {

    const auto component = static_cast<Component *>(c);
    component->logger->setDebugLogging(loggingOn);
    return fmi2OK;
}

fmi2Status fmi2SetRealInputDerivatives(fmi2Component,
                                       const fmi2ValueReference[], size_t,
                                       const fmi2Integer[],
                                       const fmi2Real[]) {
    return fmi2Error;
}

fmi2Status fmi2GetRealOutputDerivatives(fmi2Component,
                                        const fmi2ValueReference[], size_t,
                                        const fmi2Integer[],
                                        fmi2Real[]) {
    return fmi2Error;
}


fmi2Status fmi2GetDirectionalDerivative(fmi2Component,
                                        const fmi2ValueReference[], size_t,
                                        const fmi2ValueReference[], size_t,
                                        const fmi2Real[],
                                        fmi2Real[]) {

    return fmi2Error;
}


fmi2Status fmi2GetFMUstate(fmi2Component c, fmi2FMUstate *state) {
    const auto component = static_cast<Component *>(c);

    if (auto s = component->slave->getFMUState()) {
        state = &s;
        return fmi2OK;
    }

    return fmi2Error;
}

fmi2Status fmi2SetFMUstate(fmi2Component c, fmi2FMUstate state) {
    const auto component = static_cast<Component *>(c);

    if (component->slave->setFmuState(state)) {
        return fmi2OK;
    }

    return fmi2Error;
}


fmi2Status fmi2FreeFMUstate(fmi2Component c, fmi2FMUstate *state) {
    const auto component = static_cast<Component *>(c);

    if (component->slave->freeFmuState(state)) {
        return fmi2OK;
    }

    return fmi2Error;
}

fmi2Status fmi2SerializedFMUstateSize(fmi2Component, fmi2FMUstate, size_t *) {

    return fmi2Error;
}

fmi2Status fmi2SerializeFMUstate(fmi2Component, fmi2FMUstate, fmi2Byte[], size_t) {

    return fmi2Error;
}

fmi2Status fmi2DeSerializeFMUstate(fmi2Component, const fmi2Byte[], size_t, fmi2FMUstate *) {

    return fmi2Error;
}

void fmi2FreeInstance(fmi2Component c) {
    const auto component = static_cast<Component *>(c);
    delete component;
}
}
