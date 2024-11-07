
#include "fmi2/fmi2Functions.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

#include "fmu4cpp/fmu_base.hpp"
#include "fmu4cpp/logger.hpp"

namespace {

    // A struct that holds all the data for one model instance.
    struct Component {

        Component(std::unique_ptr<fmu4cpp::fmu_base> slave, const fmi2CallbackFunctions &callbackFunctions)
            : lastSuccessfulTime{std::numeric_limits<double>::quiet_NaN()},
              slave(std::move(slave)),
              logger(this, callbackFunctions, this->slave->instanceName()) {

            this->slave->__set_logger(&logger);
        }

        double lastSuccessfulTime;
        std::unique_ptr<fmu4cpp::fmu_base> slave;
        fmu4cpp::logger logger;
    };

}// namespace

extern "C" {

const char *fmi2GetTypesPlatform() {
    return fmi2TypesPlatform;
}

const char *fmi2GetVersion(void) {
    return "2.0";
}

void write_description(const char *location) {
    const auto instance = fmu4cpp::createInstance("", "");
    const auto xml = instance->make_description();
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
        fmu4cpp::logger l(nullptr, *functions, instanceName);
        l.log(fmi2Fatal, "", "Error. Wrong guid!");
        return nullptr;
    }

    const auto c = new Component(std::move(slave), *functions);
    c->logger.setDebugLogging(loggingOn);
    return c;
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
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
        return fmi2Error;
    }
}

fmi2Status fmi2EnterInitializationMode(fmi2Component c) {
    const auto component = static_cast<Component *>(c);

    try {
        component->slave->enter_initialisation_mode();
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
        return fmi2Error;
    }
}

fmi2Status fmi2ExitInitializationMode(fmi2Component c) {
    const auto component = static_cast<Component *>(c);
    try {
        component->slave->exit_initialisation_mode();
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
        return fmi2Error;
    }
}

fmi2Status fmi2Terminate(fmi2Component c) {
    const auto component = static_cast<Component *>(c);
    try {
        component->slave->terminate();
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
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
        const double endTime = currentCommunicationPoint;
        const auto ok = component->slave->do_step(
                currentCommunicationPoint,
                communicationStepSize);
        if (ok) {
            component->lastSuccessfulTime = currentCommunicationPoint + communicationStepSize;
            return fmi2OK;
        } else {
            component->lastSuccessfulTime = endTime;
            return fmi2Discard;
        }
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
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
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
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
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
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
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
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
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
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
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
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
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
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
    } catch (const fmu4cpp::fatal_error &) {
        return fmi2Fatal;
    } catch (const std::exception &) {
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
        return fmi2Fatal;
    } catch (const std::exception &ex) {
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
                               size_t nCategories,
                               const fmi2String categories[]) {

    const auto component = static_cast<Component *>(c);
    component->logger.setDebugLogging(loggingOn);
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


fmi2Status fmi2GetFMUstate(fmi2Component, fmi2FMUstate *) {

    return fmi2Error;
}

fmi2Status fmi2SetFMUstate(fmi2Component, fmi2FMUstate) {

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

fmi2Status fmi2FreeFMUstate(fmi2Component, fmi2FMUstate *) {

    return fmi2Error;
}

void fmi2FreeInstance(fmi2Component c) {
    const auto component = static_cast<Component *>(c);
    delete component;
}
}
