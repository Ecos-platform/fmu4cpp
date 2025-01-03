
#include "catch2/matchers/catch_matchers_vector.hpp"
#include "fmi2/fmi2Functions.h"


#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdarg>
#include <iostream>

#include <fmu4cpp/fmu_base.hpp>

class Model : public fmu4cpp::fmu_base {

public:
    Model(const std::string &instanceName, const std::filesystem::path &resources)
        : fmu_base(instanceName, resources), reals_(4) {

        for (int i = 0; i < reals_.size(); i++) {
            register_variable(
                    real("reals" + std::to_string(i), [this, i] { return reals_[i]; }, [this, i](double val) { reals_[i] = val; }).setCausality(fmu4cpp::causality_t::PARAMETER).setVariability(fmu4cpp::variability_t::TUNABLE));
        }

        Model::reset();
    }

    bool do_step(double currentTime, double dt) override {
        return true;
    }

    void reset() override {
        reals_.assign({1, 2, 3, 4});
    }

private:
    std::vector<double> reals_;
};

fmu4cpp::model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Array";
    info.description = "A simple model with arrays";
    info.modelIdentifier = FMU4CPP_MODEL_IDENTIFIER;
    return info;
}

std::unique_ptr<fmu4cpp::fmu_base> fmu4cpp::createInstance(const std::string &instanceName,
                                                           const std::filesystem::path &fmuResourceLocation) {
    return std::make_unique<Model>(instanceName, fmuResourceLocation);
}

void fmilogger(fmi2Component, fmi2String instanceName, fmi2Status status, fmi2String category, fmi2String message, ...) {
    va_list args;
    va_start(args, message);
    char msgstr[1024];
    sprintf(msgstr, "%i: [%s] %s\n", status, instanceName, message);
    printf(msgstr, args);
    va_end(args);
}

TEST_CASE("test_array") {

    Model model("", "");
    const auto guid = model.guid();

    fmi2CallbackFunctions callbackFunctions;
    callbackFunctions.logger = &fmilogger;

    auto c = fmi2Instantiate("array", fmi2CoSimulation, guid.c_str(), "", &callbackFunctions, false, true);
    REQUIRE(c);

    REQUIRE(fmi2EnterInitializationMode(c) == fmi2OK);
    REQUIRE(fmi2ExitInitializationMode(c) == fmi2OK);
    REQUIRE(fmi2SetupExperiment(c, false, 0, 0, false, 0) == fmi2OK);

    std::vector<double> values(4);
    for (int i = 0; i < 4; i++) {
        fmi2ValueReference ref = i;
        fmi2GetReal(c, &ref, 1, &values[i]);
    }
    REQUIRE(values == std::vector<double>{1, 2, 3, 4});

    for (int i = 0; i < 4; i++) {
        fmi2ValueReference ref = i;
        fmi2Real val = 9;
        fmi2SetReal(c, &ref, 1, &val);
    }

    for (int i = 0; i < 4; i++) {
        fmi2ValueReference ref = i;
        fmi2GetReal(c, &ref, 1, &values[i]);
    }
    REQUIRE(values == std::vector<double>{9, 9, 9, 9});


    REQUIRE(fmi2Terminate(c) == fmi2OK);

    fmi2FreeInstance(c);
}
