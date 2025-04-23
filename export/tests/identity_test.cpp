
#include "catch2/matchers/catch_matchers_vector.hpp"
#include "fmi2/fmi2Functions.h"


#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdarg>

#include <fmu4cpp/fmu_base.hpp>

class Model : public fmu4cpp::fmu_base {

public:
    Model(const std::string &instanceName, const std::filesystem::path &resources)
        : fmu_base(instanceName, resources) {

        register_variable(integer("integerIn", &integer_)
                                  .setCausality(fmu4cpp::causality_t::INPUT)
                                  .setVariability(fmu4cpp::variability_t::DISCRETE));

        register_variable(real("realIn", &real_)
                                  .setCausality(fmu4cpp::causality_t::INPUT)
                                  .setVariability(fmu4cpp::variability_t::DISCRETE));

        register_variable(boolean("booleanIn", &boolean_)
                                  .setCausality(fmu4cpp::causality_t::INPUT)
                                  .setVariability(fmu4cpp::variability_t::DISCRETE));

        register_variable(string("stringIn", &string_)
                                  .setCausality(fmu4cpp::causality_t::INPUT)
                                  .setVariability(fmu4cpp::variability_t::DISCRETE));


        register_variable(integer("integerOut", &integer_)
                                  .setCausality(fmu4cpp::causality_t::OUTPUT)
                                  .setVariability(fmu4cpp::variability_t::DISCRETE)
                                  .setInitial(fmu4cpp::initial_t::CALCULATED)
                                  .setDependencies({get_int_variable("integerIn")->index()}));

        register_variable(real("realOut", &real_)
                                  .setCausality(fmu4cpp::causality_t::OUTPUT)
                                  .setVariability(fmu4cpp::variability_t::DISCRETE)
                                  .setInitial(fmu4cpp::initial_t::CALCULATED)
                                  .setDependencies({get_real_variable("realIn")->index()}));

        register_variable(boolean("booleanOut", &boolean_)
                                  .setCausality(fmu4cpp::causality_t::OUTPUT)
                                  .setVariability(fmu4cpp::variability_t::DISCRETE)
                                  .setInitial(fmu4cpp::initial_t::CALCULATED)
                                  .setDependencies({get_bool_variable("booleanIn")->index()}));

        register_variable(string("stringOut", [this] { return string_; })
                                  .setCausality(fmu4cpp::causality_t::OUTPUT)
                                  .setVariability(fmu4cpp::variability_t::DISCRETE)
                                  .setInitial(fmu4cpp::initial_t::CALCULATED)
                                  .setDependencies({get_string_variable("stringIn")->index()}));

        Model::reset();
    }

    bool do_step(double currentTime, double dt) override {
        return true;
    }

    void reset() override {
        integer_ = 0;
        real_ = 0;
        boolean_ = false;
        string_ = "empty";
    }

private:
    int integer_;
    double real_;
    bool boolean_;
    std::string string_;
};

fmu4cpp::model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Identity";
    info.description = "A simple feed-trough model";
    info.modelIdentifier = FMU4CPP_MODEL_IDENTIFIER;
    return info;
}

std::unique_ptr<fmu4cpp::fmu_base> fmu4cpp::createInstance(const std::string &instanceName,
                                                           const std::filesystem::path &fmuResourceLocation) {
    return std::make_unique<Model>(instanceName, fmuResourceLocation);
}

int readInt(fmi2Component c) {
    fmi2ValueReference ref = 1;
    fmi2Integer value;
    REQUIRE(fmi2GetInteger(c, &ref, 1, &value) == fmi2OK);

    return value;
}

void setInt(fmi2Component c, int value) {
    fmi2ValueReference ref = 0;
    REQUIRE(fmi2SetInteger(c, &ref, 1, &value) == fmi2OK);
}

double readReal(fmi2Component c) {
    fmi2ValueReference ref = 2;
    fmi2Real value;
    REQUIRE(fmi2GetReal(c, &ref, 1, &value) == fmi2OK);

    return value;
}

void setReal(fmi2Component c, double value) {
    fmi2ValueReference ref = 1;
    REQUIRE(fmi2SetReal(c, &ref, 1, &value) == fmi2OK);
}

bool readBool(fmi2Component c) {
    fmi2ValueReference ref = 1;
    fmi2Boolean value;
    REQUIRE(fmi2GetBoolean(c, &ref, 1, &value) == fmi2OK);

    return value;
}

void setBool(fmi2Component c, bool value) {
    fmi2ValueReference ref = 0;
    fmi2Boolean value_ = value;
    REQUIRE(fmi2SetBoolean(c, &ref, 1, &value_) == fmi2OK);
}


std::string readString(fmi2Component c) {
    fmi2ValueReference ref = 1;
    fmi2String value;
    REQUIRE(fmi2GetString(c, &ref, 1, &value) == fmi2OK);

    return value;
}

void setString(fmi2Component c, const std::string &value) {
    fmi2ValueReference ref = 0;
    fmi2String value_ = value.c_str();
    REQUIRE(fmi2SetString(c, &ref, 1, &value_) == fmi2OK);
}

void setOutputFail(fmi2Component c) {
    fmi2ValueReference ref = 1;         // out of bounds
    fmi2ValueReference real_ref = 2;    // out of bounds
    fmi2Integer i = 0;
    fmi2String s = "";
    fmi2Real r = 0;
    fmi2Boolean b = fmi2False;

    REQUIRE(fmi2SetInteger(c, &ref, 1, &i) == fmi2Error);
    REQUIRE(fmi2SetReal(c, &real_ref, 1, &r) == fmi2Error);
    REQUIRE(fmi2SetString(c, &ref, 1, &s) == fmi2Error);
    REQUIRE(fmi2SetBoolean(c, &ref, 1, &b) == fmi2Error);
}

void fmilogger(fmi2Component, fmi2String instanceName, fmi2Status status, fmi2String /*category*/, fmi2String message, ...) {
    va_list args;
    va_start(args, message);
    char msgstr[1024];
    sprintf(msgstr, "%i: [%s] %s\n", status, instanceName, message);
    printf(msgstr, args);
    va_end(args);
}

TEST_CASE("test_identity") {

    Model model("", "");
    const auto guid = model.guid();

    fmi2CallbackFunctions callbackFunctions;
    callbackFunctions.logger = &fmilogger;

    auto c = fmi2Instantiate("identity", fmi2CoSimulation, guid.c_str(), "", &callbackFunctions, false, true);
    REQUIRE(c);

    REQUIRE(fmi2EnterInitializationMode(c) == fmi2OK);
    REQUIRE(fmi2ExitInitializationMode(c) == fmi2OK);
    REQUIRE(fmi2SetupExperiment(c, false, 0, 0, false, 0) == fmi2OK);

    REQUIRE(readReal(c) == Catch::Approx(0));
    REQUIRE(readString(c) == "empty");
    REQUIRE(readInt(c) == 0);
    REQUIRE(readBool(c) == false);


    double t{0};
    const double dt{0.1};

    bool b{false};
    int counter{0};
    while (t < 1) {

        setInt(c, counter);
        setReal(c, t);
        setString(c, std::to_string(t));
        setBool(c, b);
        REQUIRE(fmi2DoStep(c, t, dt, true) == fmi2OK);
        REQUIRE(readReal(c) == Catch::Approx(t));
        REQUIRE(readString(c) == std::to_string(t));
        REQUIRE(readInt(c) == counter);
        REQUIRE(readBool(c) == b);

        t += dt;
        counter++;
        b = !b;
    }

    setOutputFail(c);

    REQUIRE(fmi2Terminate(c) == fmi2OK);

    fmi2FreeInstance(c);
}
