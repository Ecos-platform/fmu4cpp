
#include "catch2/matchers/catch_matchers_vector.hpp"
#include "fmi3/fmi3Functions.h"


#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdarg>
#include <iostream>

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


int readInt(fmi3Instance c, fmi3ValueReference ref) {
    fmi3Int32 value;
    REQUIRE(fmi3GetInt32(c, &ref, 1, &value, 1) == fmi3OK);

    return value;
}

void setInt(fmi3Instance c, fmi3ValueReference ref, int value) {
    REQUIRE(fmi3SetInt32(c, &ref, 1, &value, 1) == fmi3OK);
}

double readReal(fmi3Instance c, fmi3ValueReference ref) {
    fmi3Float64 value;
    REQUIRE(fmi3GetFloat64(c, &ref, 1, &value, 0) == fmi3OK);

    return value;
}

void setReal(fmi3Instance c, fmi3ValueReference ref, double value) {
    REQUIRE(fmi3SetFloat64(c, &ref, 1, &value, 0) == fmi3OK);
}

bool readBool(fmi3Instance c, fmi3ValueReference ref) {
    fmi3Boolean value;
    REQUIRE(fmi3GetBoolean(c, &ref, 1, &value, 0) == fmi3OK);

    return value;
}

void setBool(fmi3Instance c, fmi3ValueReference ref, bool value) {
    fmi3Boolean value_ = value;
    REQUIRE(fmi3SetBoolean(c, &ref, 1, &value_, 0) == fmi3OK);
}


std::string readString(fmi3Instance c, fmi3ValueReference ref) {
    fmi3String value;
    REQUIRE(fmi3GetString(c, &ref, 1, &value, 0) == fmi3OK);

    return value;
}

void setString(fmi3Instance c, fmi3ValueReference ref, const std::string &value) {
    fmi3String value_ = value.c_str();
    REQUIRE(fmi3SetString(c, &ref, 1, &value_, 0) == fmi3OK);
}

void setOutputFail(fmi3Instance c) {
    fmi3ValueReference ref = 999;// out of bounds
    fmi3Int32 i = 0;
    fmi3String s = "";
    fmi3Float64 r = 0;
    fmi3Boolean b = fmi3False;

    REQUIRE(fmi3SetInt32(c, &ref, 1, &i, 0) == fmi3Error);
    REQUIRE(fmi3SetFloat64(c, &ref, 1, &r, 0) == fmi3Error);
    REQUIRE(fmi3SetString(c, &ref, 1, &s, 0) == fmi3Error);
    REQUIRE(fmi3SetBoolean(c, &ref, 1, &b, 0) == fmi3Error);
}

void fmilogger(fmi3InstanceEnvironment, fmi3Status status, fmi3String /*category*/, fmi3String message) {

    std::cout << status << ": " << message << std::endl;
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

    const auto c = fmi3InstantiateCoSimulation("identity", guid.c_str(), "", false, true, false, false, nullptr, 0, nullptr, fmilogger, nullptr);
    REQUIRE(c);

    REQUIRE(fmi3EnterInitializationMode(c, false, 0, 0, false, 0) == fmi3OK);
    REQUIRE(fmi3ExitInitializationMode(c) == fmi3OK);

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

        bool eventhandlingNeeded;
        bool terminateSimulation;
        bool earlyReturn;
        double lastSucessfulTime;

        REQUIRE(fmi3DoStep(c, t, dt, true, &eventhandlingNeeded, &terminateSimulation, &earlyReturn, &lastSucessfulTime) == fmi3OK);

        REQUIRE(readReal(c, realOut->value_reference()) == Catch::Approx(t));
        REQUIRE(readString(c, stringOut->value_reference()) == std::to_string(t));
        REQUIRE(readInt(c, integerOut->value_reference()) == counter);
        REQUIRE(readBool(c, booleanOut->value_reference()) == b);

        t += dt;
        counter++;
        b = !b;
    }

    setOutputFail(c);

    REQUIRE(fmi3Terminate(c) == fmi3OK);

    fmi3FreeInstance(c);
}
