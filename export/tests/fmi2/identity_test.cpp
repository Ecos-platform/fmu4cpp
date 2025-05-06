
#include "catch2/matchers/catch_matchers_vector.hpp"
#include "fmi2/fmi2Functions.h"


#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdarg>

#include <fmu4cpp/fmu_base.hpp>

class Model : public fmu4cpp::fmu_base {

public:
    explicit Model(const fmu4cpp::fmu_data &data) : fmu_base(data) {

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
                                  .setDependencies({"integerIn"}));

        register_variable(real("realOut", &real_)
                                  .setCausality(fmu4cpp::causality_t::OUTPUT)
                                  .setVariability(fmu4cpp::variability_t::DISCRETE)
                                  .setInitial(fmu4cpp::initial_t::CALCULATED)
                                  .setDependencies({"realIn"}));

        register_variable(boolean("booleanOut", &boolean_)
                                  .setCausality(fmu4cpp::causality_t::OUTPUT)
                                  .setVariability(fmu4cpp::variability_t::DISCRETE)
                                  .setInitial(fmu4cpp::initial_t::CALCULATED)
                                  .setDependencies({"booleanIn"}));

        register_variable(string("stringOut", [this] { return string_; })
                                  .setCausality(fmu4cpp::causality_t::OUTPUT)
                                  .setVariability(fmu4cpp::variability_t::DISCRETE)
                                  .setInitial(fmu4cpp::initial_t::CALCULATED)
                                  .setDependencies({"stringIn"}));

        Model::reset();
    }

    bool do_step(double dt) override {
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

FMU4CPP_INSTANTIATE(Model);


int readInt(fmi2Component c, fmi2ValueReference ref) {
    fmi2Integer value;
    REQUIRE(fmi2GetInteger(c, &ref, 1, &value) == fmi2OK);

    return value;
}

void setInt(fmi2Component c, fmi2ValueReference ref, int value) {
    REQUIRE(fmi2SetInteger(c, &ref, 1, &value) == fmi2OK);
}

double readReal(fmi2Component c, fmi2ValueReference ref) {
    fmi2Real value;
    REQUIRE(fmi2GetReal(c, &ref, 1, &value) == fmi2OK);

    return value;
}

void setReal(fmi2Component c, fmi2ValueReference ref, double value) {
    REQUIRE(fmi2SetReal(c, &ref, 1, &value) == fmi2OK);
}

bool readBool(fmi2Component c, fmi2ValueReference ref) {
    fmi2Boolean value;
    REQUIRE(fmi2GetBoolean(c, &ref, 1, &value) == fmi2OK);

    return value;
}

void setBool(fmi2Component c, fmi2ValueReference ref, bool value) {
    fmi2Boolean value_ = value;
    REQUIRE(fmi2SetBoolean(c, &ref, 1, &value_) == fmi2OK);
}


std::string readString(fmi2Component c, fmi2ValueReference ref) {
    fmi2String value;
    REQUIRE(fmi2GetString(c, &ref, 1, &value) == fmi2OK);

    return value;
}

void setString(fmi2Component c, fmi2ValueReference ref, const std::string &value) {
    fmi2String value_ = value.c_str();
    REQUIRE(fmi2SetString(c, &ref, 1, &value_) == fmi2OK);
}

void setOutputFail(fmi2Component c) {
    fmi2ValueReference ref = 999;// out of bounds
    fmi2Integer i = 0;
    fmi2String s = "";
    fmi2Real r = 0;
    fmi2Boolean b = fmi2False;

    REQUIRE(fmi2SetInteger(c, &ref, 1, &i) == fmi2Error);
    REQUIRE(fmi2SetReal(c, &ref, 1, &r) == fmi2Error);
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

    Model model({});
    const auto guid = model.guid();

    const auto realIn = model.get_real_variable("realIn");
    const auto stringIn = model.get_string_variable("stringIn");
    const auto integerIn = model.get_int_variable("integerIn");
    const auto booleanIn = model.get_bool_variable("booleanIn");

    const auto realOut = model.get_real_variable("realOut");
    const auto stringOut = model.get_string_variable("stringOut");
    const auto integerOut = model.get_int_variable("integerOut");
    const auto booleanOut = model.get_bool_variable("booleanOut");

    fmi2CallbackFunctions callbackFunctions;
    callbackFunctions.logger = &fmilogger;

    const auto c = fmi2Instantiate("identity", fmi2CoSimulation, guid.c_str(), "", &callbackFunctions, false, true);
    REQUIRE(c);

    REQUIRE(fmi2SetupExperiment(c, false, 0, 0, false, 0) == fmi2OK);
    REQUIRE(fmi2EnterInitializationMode(c) == fmi2OK);
    REQUIRE(fmi2ExitInitializationMode(c) == fmi2OK);

    REQUIRE(readReal(c, realOut->value_reference()) == Catch::Approx(0));
    REQUIRE(readString(c, stringOut->value_reference()) == "empty");
    REQUIRE(readInt(c, integerOut->value_reference()) == 0);
    REQUIRE(readBool(c, booleanOut->value_reference()) == false);

    double t{0};
    const double dt{0.1};

    bool b{false};
    int counter{0};
    while (t < 1) {

        setInt(c, integerIn->value_reference(), counter);
        setReal(c, realIn->value_reference(), t);
        setString(c, stringIn->value_reference(), std::to_string(t));
        setBool(c, booleanIn->value_reference(), b);

        REQUIRE(fmi2DoStep(c, t, dt, true) == fmi2OK);

        REQUIRE(readReal(c, realOut->value_reference()) == Catch::Approx(t));
        REQUIRE(readString(c, stringOut->value_reference()) == std::to_string(t));
        REQUIRE(readInt(c, integerOut->value_reference()) == counter);
        REQUIRE(readBool(c, booleanOut->value_reference()) == b);

        t += dt;
        counter++;
        b = !b;
    }

    setOutputFail(c);

    REQUIRE(fmi2Terminate(c) == fmi2OK);

    fmi2FreeInstance(c);
}
