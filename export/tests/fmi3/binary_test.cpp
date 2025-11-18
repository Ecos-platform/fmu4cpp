
#include "fmi3/fmi3Functions.h"

#include <nlohmann/json.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <cstdarg>
#include <iostream>

#include <fmu4cpp/fmu_base.hpp>

class Model : public fmu4cpp::fmu_base {

public:
    FMU4CPP_CTOR(Model) {

        register_variable(binary("binaryIn", &binaryIn_).setCausality(fmu4cpp::causality_t::INPUT));
        register_variable(real("numberOut", &numberOut_).setCausality(fmu4cpp::causality_t::OUTPUT));

        Model::reset();
    }

    bool do_step(double dt) override {

        std::cout << binaryIn_ << std::endl;
        nlohmann::json j = nlohmann::json::from_msgpack(binaryIn_);
        numberOut_ = j["number"];

        return true;
    }

    void reset() override {
        binaryIn_ = "";
        numberOut_ = 0;
    }

private:
    std::string binaryIn_;
    double numberOut_;
};

fmu4cpp::model_info fmu4cpp::get_model_info() {
    return {};
}

std::string fmu4cpp::model_identifier() {
    return "binary";
}

FMU4CPP_INSTANTIATE(Model);


void fmilogger(fmi3InstanceEnvironment, fmi3Status status, fmi3String /*category*/, fmi3String message) {

    std::cout << status << ": " << message << std::endl;
}


TEST_CASE("test_binary") {

    Model model({});
    const auto guid = model.guid();

    auto c = fmi3InstantiateCoSimulation(fmu4cpp::model_identifier().c_str(), guid.c_str(), "", false, true, false, false, nullptr, 0, nullptr, fmilogger, nullptr);
    REQUIRE(c);

    REQUIRE(fmi3EnterInitializationMode(c, false, 0, 0, false, 0) == fmi3OK);
    REQUIRE(fmi3ExitInitializationMode(c) == fmi3OK);

    double numberIn = 3.9;

    nlohmann::json j;
    j["number"] = numberIn;

    const auto data = nlohmann::json::to_msgpack(j);
    const uint8_t *binaryPtr = data.data();

    fmi3ValueReference inVr = 1;
    size_t valueSize = data.size();

    fmi3SetBinary(
            c,
            &inVr,     // array of value references
            1,         // nValueReferences
            &valueSize,// array of sizes
            &binaryPtr,// array of binary pointers
            1          // nValues
    );


    bool eventhandlingNeeded;
    bool terminateSimulation;
    bool earlyReturn;
    double lastSucessfulTime;

    REQUIRE(fmi3DoStep(c, 0, 0.1, true, &eventhandlingNeeded, &terminateSimulation, &earlyReturn, &lastSucessfulTime) == fmi3OK);


    fmi3ValueReference outVr = 2;
    double numberOut;
    fmi3GetFloat64(c, &outVr, 1, &numberOut, 1);

    CHECK(numberOut == Catch::Approx(3.9));

    REQUIRE(fmi3Terminate(c) == fmi3OK);

    fmi3FreeInstance(c);
}
