
#include "fmi2/fmi2Functions.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

#include "fmu_base.hpp"

namespace {

    // A struct that holds all the data for one model instance.
    struct Component {

        Component(std::unique_ptr<fmu4cpp::fmu_base> slave, fmi2CallbackFunctions callbackFunctions)
            : lastSuccessfulTime{std::numeric_limits<double>::quiet_NaN()}, callbackFunctions(callbackFunctions), slave(std::move(slave)) {}

        // Co-simulation
        double lastSuccessfulTime;
        fmi2CallbackFunctions callbackFunctions;
        std::unique_ptr<fmu4cpp::fmu_base> slave;
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
    auto instance = fmu4cpp::createInstance("", "");
    auto xml = instance->make_description();
    std::ofstream of(location);
    of << xml;
    of.close();
}

fmi2Component fmi2Instantiate(fmi2String instanceName,
                              fmi2Type fmuType,
                              fmi2String fmuGUID,
                              fmi2String fmuResourceLocation,
                              const fmi2CallbackFunctions *functions,
                              fmi2Boolean visible,
                              fmi2Boolean loggingOn) {

    if (fmuType != fmi2CoSimulation) {
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
    return new Component(std::move(slave), *functions);
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

    auto component = reinterpret_cast<Component *>(c);
    try {
        component->slave->setup_experiment(startTime, stop, tol);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        return fmi2Error;
    }
}

fmi2Status fmi2EnterInitializationMode(fmi2Component c) {
    auto component = reinterpret_cast<Component *>(c);

    try {
        component->slave->enter_initialisation_mode();
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        return fmi2Error;
    }
}

fmi2Status fmi2ExitInitializationMode(fmi2Component c) {
    auto component = reinterpret_cast<Component *>(c);
    try {
        component->slave->exit_initialisation_mode();
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        return fmi2Error;
    }
}

fmi2Status fmi2Terminate(fmi2Component c) {
    auto component = reinterpret_cast<Component *>(c);
    try {
        component->slave->terminate();
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        return fmi2Error;
    }
}

fmi2Status fmi2DoStep(
        fmi2Component c,
        fmi2Real currentCommunicationPoint,
        fmi2Real communicationStepSize,
        fmi2Boolean /*noSetFMUStatePriorToCurrentPoint*/) {
    auto component = reinterpret_cast<Component *>(c);
    try {
        double endTime = currentCommunicationPoint;
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
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &e) {
        return fmi2Error;
    }
}

fmi2Status fmi2CancelStep(fmi2Component c) {
    return fmi2Error;
}

fmi2Status fmi2Reset(fmi2Component c) {
    auto component = reinterpret_cast<Component *>(c);
    component->slave->reset();
    return fmi2OK;
}

fmi2Status fmi2GetInteger(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        fmi2Integer value[]) {
    const auto component = reinterpret_cast<Component *>(c);
    try {
        component->slave->get_integer(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        return fmi2Error;
    }
}

fmi2Status fmi2GetReal(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        fmi2Real value[]) {
    const auto component = reinterpret_cast<Component *>(c);
    try {
        component->slave->get_real(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        return fmi2Error;
    }
}

fmi2Status fmi2GetBoolean(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        fmi2Boolean value[]) {
    const auto component = reinterpret_cast<Component *>(c);
    try {
        component->slave->get_boolean(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        return fmi2Error;
    }
}

fmi2Status fmi2GetString(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        fmi2String value[]) {
    const auto component = reinterpret_cast<Component *>(c);
    try {
        component->slave->get_string(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        return fmi2Error;
    }
}

fmi2Status fmi2SetInteger(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        const fmi2Integer value[]) {
    const auto component = reinterpret_cast<Component *>(c);
    try {
        component->slave->set_integer(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        return fmi2Error;
    }
}

fmi2Status fmi2SetReal(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        const fmi2Real value[]) {
    const auto component = reinterpret_cast<Component *>(c);
    try {
        component->slave->set_real(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        return fmi2Error;
    }
}

fmi2Status fmi2SetBoolean(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        const fmi2Boolean value[]) {
    const auto component = reinterpret_cast<Component *>(c);
    try {
        component->slave->set_boolean(vr, nvr, value);
        return fmi2OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        return fmi2Fatal;
    } catch (const std::exception &ex) {
        return fmi2Error;
    }
}

fmi2Status fmi2SetString(
        fmi2Component c,
        const fmi2ValueReference vr[],
        size_t nvr,
        const fmi2String value[]) {
    const auto component = reinterpret_cast<Component *>(c);
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
        fmi2Component c,
        const fmi2StatusKind,
        fmi2Status *) {
    return fmi2Error;
}

fmi2Status fmi2GetRealStatus(
        fmi2Component c,
        const fmi2StatusKind s,
        fmi2Real *value) {
    const auto component = reinterpret_cast<Component *>(c);
    if (s == fmi2LastSuccessfulTime) {
        *value = component->lastSuccessfulTime;
        return fmi2OK;
    } else {
        return fmi2Error;
    }
}

fmi2Status fmi2GetIntegerStatus(
        fmi2Component c,
        const fmi2StatusKind,
        fmi2Integer *) {
    return fmi2Error;
}

fmi2Status fmi2GetBooleanStatus(
        fmi2Component c,
        const fmi2StatusKind,
        fmi2Boolean *) {
    return fmi2Error;
}

fmi2Status fmi2GetStringStatus(
        fmi2Component c,
        const fmi2StatusKind,
        fmi2String *) {
    return fmi2Error;
}

void fmi2FreeInstance(fmi2Component c) {
    auto component = reinterpret_cast<Component *>(c);
    delete (component);
}
}
