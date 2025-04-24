
#include "fmi3/fmi3Functions.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

#include "fmu4cpp/fmu_base.hpp"
#include "fmu4cpp/fmu_except.hpp"
#include "fmu4cpp/logger.hpp"

namespace {

    fmiStatus toCommonStatusFromFmi3(fmi3Status status) {
        switch (status) {
            case fmi3OK:
                return fmiOK;
            case fmi3Warning:
                return fmiWarning;
            case fmi3Discard:
                return fmiDiscard;
            case fmi3Error:
                return fmiError;
            case fmi3Fatal:
                return fmiFatal;
            default:
                return fmiStatusUnknown;
        }
    }

    fmi3Status toFmi3StatusFromCommon(fmiStatus status) {
        switch (status) {
            case fmiOK:
                return fmi3OK;
            case fmiWarning:
                return fmi3Warning;
            case fmiDiscard:
                return fmi3Discard;
            case fmiError:
                return fmi3Error;
            case fmiFatal:
                return fmi3Fatal;
            default:
                return fmi3Error;// or another appropriate fallback
        }
    }

    class fmi3Logger : public fmu4cpp::logger {

    public:
        fmi3Logger(fmi3InstanceEnvironment env, fmi3LogMessageCallback logCallback, std::string instanceName)
            : logger(std::move(instanceName)), env_(env), logCallback_(logCallback) {}

        void debugLog(fmiStatus s, const std::string &message) override {
            const std::string msg = instanceName_ + ": " + message;
            logCallback_(env_, toFmi3StatusFromCommon(s), nullptr, msg.c_str());
        }

    private:
        fmi3InstanceEnvironment env_;
        fmi3LogMessageCallback logCallback_;
    };

    // A struct that holds all the data for one model instance.
    struct Fmi3Component {

        Fmi3Component(std::unique_ptr<fmu4cpp::fmu_base> slave, fmi3InstanceEnvironment env, fmi3LogMessageCallback logCallback)
            : slave(std::move(slave)),
              logger(std::make_unique<fmi3Logger>(env, logCallback, this->slave->instanceName())) {

            this->slave->__set_logger(logger.get());
        }

        std::unique_ptr<fmu4cpp::fmu_base> slave;
        std::unique_ptr<fmu4cpp::logger> logger;
    };

}// namespace

extern "C" {

const char *fmi3GetVersion(void) {
    return "3.0";
}

FMI3_Export void write_description(const char *location) {
    const auto instance = fmu4cpp::createInstance("", "");
    const auto xml = instance->make_description_v3();
    std::ofstream of(location);
    of << xml;
    of.close();
}

fmi3Instance fmi3InstantiateModelExchange(fmi3String instanceName,
                                          fmi3String instantiationToken,
                                          fmi3String resourcePath,
                                          fmi3Boolean visible,
                                          fmi3Boolean loggingOn,
                                          fmi3InstanceEnvironment instanceEnvironment,
                                          fmi3LogMessageCallback logMessage) {

    std::cerr << "[fmu4cpp] Unsupported mode: Model Exchange" << std::endl;
    fmi3Logger l(instanceEnvironment, logMessage, instanceName);
    l.log(fmiFatal, "Unsupported mode: Model Exchange");
    return nullptr;
}

fmi3Instance fmi3InstantiateScheduledExecution(
        fmi3String instanceName,
        fmi3String instantiationToken,
        fmi3String resourcePath,
        fmi3Boolean visible,
        fmi3Boolean loggingOn,
        fmi3InstanceEnvironment instanceEnvironment,
        fmi3LogMessageCallback logMessage,
        fmi3ClockUpdateCallback clockUpdate,
        fmi3LockPreemptionCallback lockPreemption,
        fmi3UnlockPreemptionCallback unlockPreemption) {

    std::cerr << "[fmu4cpp] Unsupported mode: Scheduled Execution" << std::endl;
    fmi3Logger l(instanceEnvironment, logMessage, instanceName);
    l.log(fmiFatal, "Unsupported mode: Scheduled Execution");
    return nullptr;
}

fmi3Instance fmi3InstantiateCoSimulation(
        fmi3String instanceName,
        fmi3String instantiationToken,
        fmi3String resourcePath,
        fmi3Boolean visible,
        fmi3Boolean loggingOn,
        fmi3Boolean eventModeUsed,
        fmi3Boolean earlyReturnAllowed,
        const fmi3ValueReference requiredIntermediateVariables[],
        size_t nRequiredIntermediateVariables,
        fmi3InstanceEnvironment instanceEnvironment,
        fmi3LogMessageCallback logMessage,
        fmi3IntermediateUpdateCallback intermediateUpdate) {

    int magic = 1;
#ifdef _MSC_VER
    magic = 0;
#endif

    std::string resources(resourcePath);

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
    if (guid != instantiationToken) {
        std::cerr << "[fmu4cpp] Error. Wrong guid!" << std::endl;
        fmi3Logger l(instanceEnvironment, logMessage, instanceName);
        l.log(fmiFatal, "Error. Wrong guid!");
        return nullptr;
    }

    auto c = std::make_unique<Fmi3Component>(std::move(slave), instanceEnvironment, logMessage);
    c->logger->setDebugLogging(loggingOn);

    return c.release();
}

fmi3Status fmi3EnterEventMode(fmi3Instance instance) {

    std::cout << "fmi3EnterEventMode" << std::endl;

    return fmi3Error;
}

fmi3Status fmi3EnterInitializationMode(fmi3Instance c,
                                       fmi3Boolean toleranceDefined,
                                       fmi3Float64 tolerance,
                                       fmi3Float64 startTime,
                                       fmi3Boolean stopTimeDefined,
                                       fmi3Float64 stopTime) {

    std::optional<double> stop;
    std::optional<double> tol;

    if (stopTimeDefined) stop = stopTime;
    if (toleranceDefined) tol = tolerance;

    const auto component = static_cast<Fmi3Component *>(c);

    try {
        component->slave->enter_initialisation_mode(startTime, stop, tol);
        return fmi3OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Error), ex.what());
        return fmi3Error;
    }
}

fmi3Status fmi3ExitInitializationMode(fmi3Instance c) {
    const auto component = static_cast<Fmi3Component *>(c);
    try {
        component->slave->exit_initialisation_mode();
        return fmi3OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Error), ex.what());
        return fmi3Error;
    }
}

fmi3Status fmi3Terminate(fmi3Instance c) {
    const auto component = static_cast<Fmi3Component *>(c);
    try {
        component->slave->terminate();
        return fmi3OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Error;
    }
}

fmi3Status fmi3DoStep(fmi3Instance c,
                      fmi3Float64 currentCommunicationPoint,
                      fmi3Float64 communicationStepSize,
                      fmi3Boolean /*noSetFMUStatePriorToCurrentPoint*/,
                      fmi3Boolean *eventHandlingNeeded,
                      fmi3Boolean *terminateSimulation,
                      fmi3Boolean *earlyReturn,
                      fmi3Float64 *lastSuccessfulTime) {

    const auto component = static_cast<Fmi3Component *>(c);
    try {
        if (component->slave->step(currentCommunicationPoint, communicationStepSize)) {
            *earlyReturn = false;
            *terminateSimulation = false;
            *eventHandlingNeeded = false;
            *lastSuccessfulTime = currentCommunicationPoint + communicationStepSize;
            return fmi3OK;
        }

        return fmi3Discard;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Error), ex.what());
        return fmi3Error;
    }
}

fmi3Status fmi3CancelStep(fmi3Instance) {
    return fmi3Error;
}

fmi3Status fmi3Reset(fmi3Instance c) {
    const auto component = static_cast<Fmi3Component *>(c);
    component->slave->reset();
    return fmi3OK;
}

fmi3Status fmi3GetInt8(fmi3Instance c,
                       const fmi3ValueReference valueReferences[],
                       size_t nValueReferences,
                       fmi3Int8 values[],
                       size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);

    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3GetInt8");
    return fmi3Error;
}

fmi3Status fmi3GetUInt8(fmi3Instance c,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        fmi3UInt8 values[],
                        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);

    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3GetUInt8");
    return fmi3Error;
}

fmi3Status fmi3GetInt16(fmi3Instance c,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        fmi3Int16 values[],
                        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);

    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3GetInt16");
    return fmi3Error;
}

fmi3Status fmi3GetUInt16(fmi3Instance c,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         fmi3UInt16 values[],
                         size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);

    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3GetUInt16");
    return fmi3Error;
}

fmi3Status fmi3GetInt32(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        fmi3Int32 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    try {
        component->slave->get_integer(vr, nvr, value);
        return fmi3OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Error), ex.what());
        return fmi3Error;
    }
}

fmi3Status fmi3GetUInt32(fmi3Instance c,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         fmi3UInt32 values[],
                         size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);

    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3GetUInt32");
    return fmi3Error;
}

fmi3Status fmi3GetInt64(fmi3Instance c,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        fmi3Int64 values[],
                        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);

    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3GetInt64");
    return fmi3Error;
}

fmi3Status fmi3GetUInt64(fmi3Instance c,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         fmi3UInt64 values[],
                         size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);

    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3GetUInt64");
    return fmi3Error;
}

fmi3Status fmi3GetFloat32(fmi3Instance c,
                          const fmi3ValueReference valueReferences[],
                          size_t nValueReferences,
                          fmi3Float32 values[],
                          size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);

    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3GetFloat32");
    return fmi3Error;
}

fmi3Status fmi3GetFloat64(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        fmi3Float64 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    try {
        component->slave->get_real(vr, nvr, value);
        return fmi3OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Error), ex.what());
        return fmi3Error;
    }
}

fmi3Status fmi3GetBoolean(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        fmi3Boolean value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    try {
        component->slave->get_boolean(vr, nvr, value);
        return fmi3OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Error), ex.what());
        return fmi3Error;
    }
}

fmi3Status fmi3GetString(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        fmi3String value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    try {
        component->slave->get_string(vr, nvr, value);
        return fmi3OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Error), ex.what());
        return fmi3Error;
    }
}
fmi3Status fmi3GetBinary(fmi3Instance c,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         size_t valueSizes[],
                         fmi3Binary values[],
                         size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);

    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3GetBinary");
    return fmi3Error;
}

fmi3Status fmi3SetBinary(fmi3Instance c,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         const size_t valueSizes[],
                         const fmi3Binary values[],
                         size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);

    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3SetBinary");
    return fmi3Error;
}

fmi3Status fmi3SetInt8(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3Int8 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupporeted function fmi3SetInt8");
    return fmi3Error;
}

fmi3Status fmi3SetUInt8(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3UInt8 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3SetUInt8");
    return fmi3Error;
}

fmi3Status fmi3SetInt16(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3Int16 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3SetInt16");
    return fmi3Error;
}

fmi3Status fmi3SetUInt16(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3UInt16 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3SetUInt16");
    return fmi3Error;
}

fmi3Status fmi3SetInt32(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3Int32 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    try {
        component->slave->set_integer(vr, nvr, value);
        return fmi3OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Error), ex.what());
        return fmi3Error;
    }
}

fmi3Status fmi3SetUInt32(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3UInt32 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3SetUInt32");
    return fmi3Error;
}

fmi3Status fmi3SetInt64(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3Int64 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupporeted function fmi3SetInt64");
    return fmi3Error;
}

fmi3Status fmi3SetUInt64(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3UInt64 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3SetUInt64");
    return fmi3Error;
}

fmi3Status fmi3SetFloat32(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3Float32 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    component->logger->log(toCommonStatusFromFmi3(fmi3Error), "Unsupported function fmi3SetFloat32");
    return fmi3Error;
}

fmi3Status fmi3SetFloat64(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3Float64 value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    try {
        component->slave->set_real(vr, nvr, value);
        return fmi3OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Error), ex.what());
        return fmi3Error;
    }
}

fmi3Status fmi3SetBoolean(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3Boolean value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    try {
        component->slave->set_boolean(vr, nvr, value);
        return fmi3OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Error), ex.what());
        return fmi3Error;
    }
}

fmi3Status fmi3SetString(
        fmi3Instance c,
        const fmi3ValueReference vr[],
        size_t nvr,
        const fmi3String value[],
        size_t nValues) {

    const auto component = static_cast<Fmi3Component *>(c);
    try {
        component->slave->set_string(vr, nvr, value);
        return fmi3OK;
    } catch (const fmu4cpp::fatal_error &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Fatal), ex.what());
        return fmi3Fatal;
    } catch (const std::exception &ex) {
        component->logger->log(toCommonStatusFromFmi3(fmi3Error), ex.what());
        return fmi3Error;
    }
}

fmi3Status fmi3SetDebugLogging(fmi3Instance c,
                               fmi3Boolean loggingOn,
                               size_t /*nCategories*/,
                               const fmi3String /*categories*/[]) {

    const auto component = static_cast<Fmi3Component *>(c);
    component->logger->setDebugLogging(loggingOn);
    return fmi3OK;
}

fmi3Status fmi3GetOutputDerivatives(fmi3Instance,
                                    const fmi3ValueReference[], size_t,
                                    const fmi3Int32[],
                                    fmi3Float64[],
                                    size_t) {
    return fmi3Error;
}


fmi3Status fmi3GetDirectionalDerivative(fmi3Instance instance,
                                        const fmi3ValueReference unknowns[],
                                        size_t nUnknowns,
                                        const fmi3ValueReference knowns[],
                                        size_t nKnowns,
                                        const fmi3Float64 seed[],
                                        size_t nSeed,
                                        fmi3Float64 sensitivity[],
                                        size_t nSensitivity) {

    return fmi3Error;
}


fmi3Status fmi3GetFMUState(fmi3Instance c, fmi3FMUState *state) {
    const auto component = static_cast<Fmi3Component *>(c);

    if (auto s = component->slave->getFMUState()) {
        state = &s;
        return fmi3OK;
    }

    return fmi3Error;
}

fmi3Status fmi3SetFMUState(fmi3Instance c, fmi3FMUState state) {
    const auto component = static_cast<Fmi3Component *>(c);

    if (component->slave->setFmuState(state)) {
        return fmi3OK;
    }

    return fmi3Error;
}


fmi3Status fmi3FreeFMUState(fmi3Instance c, fmi3FMUState *state) {
    const auto component = static_cast<Fmi3Component *>(c);

    if (component->slave->freeFmuState(state)) {
        return fmi3OK;
    }

    return fmi3Error;
}

fmi3Status fmi3SerializedFMUStateSize(fmi3Instance, fmi3FMUState, size_t *) {

    return fmi3Error;
}

fmi3Status fmi3SerializeFMUState(fmi3Instance, fmi3FMUState, fmi3Byte[], size_t) {

    return fmi3Error;
}

fmi3Status fmi3DeserializeFMUState(fmi3Instance, const fmi3Byte[], size_t, fmi3FMUState *) {

    return fmi3Error;
}

fmi3Status fmi3GetClock(fmi3Instance instance,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        fmi3Clock values[]) {
    return fmi3Error;
}

fmi3Status fmi3SetClock(fmi3Instance instance,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        const fmi3Clock values[]) {

    return fmi3Error;
}

fmi3Status fmi3GetNumberOfVariableDependencies(fmi3Instance instance,
                                               fmi3ValueReference valueReference,
                                               size_t *nDependencies) {
    return fmi3Error;
}

fmi3Status fmi3GetVariableDependencies(fmi3Instance instance,
                                       fmi3ValueReference dependent,
                                       size_t elementIndicesOfDependent[],
                                       fmi3ValueReference independents[],
                                       size_t elementIndicesOfIndependents[],
                                       fmi3DependencyKind dependencyKinds[],
                                       size_t nDependencies) {
    return fmi3Error;
}

fmi3Status fmi3GetAdjointDerivative(fmi3Instance instance,
                                    const fmi3ValueReference unknowns[],
                                    size_t nUnknowns,
                                    const fmi3ValueReference knowns[],
                                    size_t nKnowns,
                                    const fmi3Float64 seed[],
                                    size_t nSeed,
                                    fmi3Float64 sensitivity[],
                                    size_t nSensitivity) {

    return fmi3Error;
}

fmi3Status fmi3EnterConfigurationMode(fmi3Instance instance) {

    std::cout << "fmi3EnterConfigurationMode" << std::endl;

    return fmi3Error;
}

fmi3Status fmi3ExitConfigurationMode(fmi3Instance instance) {
    return fmi3Error;
}

fmi3Status fmi3GetIntervalDecimal(fmi3Instance instance,
                                  const fmi3ValueReference valueReferences[],
                                  size_t nValueReferences,
                                  fmi3Float64 intervals[],
                                  fmi3IntervalQualifier qualifiers[]) {

    return fmi3Error;
}

fmi3Status fmi3GetIntervalFraction(fmi3Instance instance,
                                   const fmi3ValueReference valueReferences[],
                                   size_t nValueReferences,
                                   fmi3UInt64 counters[],
                                   fmi3UInt64 resolutions[],
                                   fmi3IntervalQualifier qualifiers[]) {

    return fmi3Error;
}

fmi3Status fmi3GetShiftDecimal(fmi3Instance instance,
                               const fmi3ValueReference valueReferences[],
                               size_t nValueReferences,
                               fmi3Float64 shifts[]) {

    return fmi3Error;
}

fmi3Status fmi3GetShiftFraction(fmi3Instance instance,
                                const fmi3ValueReference valueReferences[],
                                size_t nValueReferences,
                                fmi3UInt64 counters[],
                                fmi3UInt64 resolutions[]) {

    return fmi3Error;
}

fmi3Status fmi3SetIntervalDecimal(fmi3Instance instance,
                                  const fmi3ValueReference valueReferences[],
                                  size_t nValueReferences,
                                  const fmi3Float64 intervals[]) {

    return fmi3Error;
}

fmi3Status fmi3SetIntervalFraction(fmi3Instance instance,
                                   const fmi3ValueReference valueReferences[],
                                   size_t nValueReferences,
                                   const fmi3UInt64 counters[],
                                   const fmi3UInt64 resolutions[]) {

    return fmi3Error;
}

fmi3Status fmi3SetShiftDecimal(fmi3Instance instance,
                               const fmi3ValueReference valueReferences[],
                               size_t nValueReferences,
                               const fmi3Float64 shifts[]) {

    return fmi3Error;
}

fmi3Status fmi3SetShiftFraction(fmi3Instance instance,
                                const fmi3ValueReference valueReferences[],
                                size_t nValueReferences,
                                const fmi3UInt64 counters[],
                                const fmi3UInt64 resolutions[]) {

    return fmi3Error;
}

fmi3Status fmi3EvaluateDiscreteStates(fmi3Instance instance) {

    return fmi3Error;
}

fmi3Status fmi3UpdateDiscreteStates(fmi3Instance instance,
                                    fmi3Boolean *discreteStatesNeedUpdate,
                                    fmi3Boolean *terminateSimulation,
                                    fmi3Boolean *nominalsOfContinuousStatesChanged,
                                    fmi3Boolean *valuesOfContinuousStatesChanged,
                                    fmi3Boolean *nextEventTimeDefined,
                                    fmi3Float64 *nextEventTime) {

    return fmi3Error;
}

fmi3Status fmi3EnterContinuousTimeMode(fmi3Instance instance) {

    std::cout << "fmi3EnterContinuousTimeMode" << std::endl;

    return fmi3Error;
}

fmi3Status fmi3CompletedIntegratorStep(fmi3Instance instance,
                                       fmi3Boolean noSetFMUStatePriorToCurrentPoint,
                                       fmi3Boolean *enterEventMode,
                                       fmi3Boolean *terminateSimulation) {

    return fmi3Error;
}

fmi3Status fmi3SetTime(fmi3Instance instance, fmi3Float64 time) {
    return fmi3Error;
}

/* tag::SetContinuousStates[] */
fmi3Status fmi3SetContinuousStates(fmi3Instance instance,
                                   const fmi3Float64 continuousStates[],
                                   size_t nContinuousStates) {
    return fmi3Error;
}
fmi3Status fmi3GetContinuousStateDerivatives(fmi3Instance instance,
                                             fmi3Float64 derivatives[],
                                             size_t nContinuousStates) {
    return fmi3Error;
}
fmi3Status fmi3GetEventIndicators(fmi3Instance instance,
                                  fmi3Float64 eventIndicators[],
                                  size_t nEventIndicators) {
    return fmi3Error;
}
fmi3Status fmi3GetContinuousStates(fmi3Instance instance,
                                   fmi3Float64 continuousStates[],
                                   size_t nContinuousStates) {
    return fmi3Error;
}
fmi3Status fmi3GetNominalsOfContinuousStates(fmi3Instance instance,
                                             fmi3Float64 nominals[],
                                             size_t nContinuousStates) {
    return fmi3Error;
}
fmi3Status fmi3GetNumberOfEventIndicators(fmi3Instance instance,
                                          size_t *nEventIndicators) {
    return fmi3Error;
}
fmi3Status fmi3GetNumberOfContinuousStates(fmi3Instance instance,
                                           size_t *nContinuousStates) {
    return fmi3Error;
}

fmi3Status fmi3EnterStepMode(fmi3Instance instance) {

    std::cout << "fmi3EnterStepMode" << std::endl;

    return fmi3Error;
}

fmi3Status fmi3ActivateModelPartition(fmi3Instance instance,
                                      fmi3ValueReference clockReference,
                                      fmi3Float64 activationTime) {

    return fmi3Error;
}

void fmi3FreeInstance(fmi3Instance c) {
    const auto component = static_cast<Fmi3Component *>(c);
    delete component;
}
}
