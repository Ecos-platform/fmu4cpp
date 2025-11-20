
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "fmi2/fmi2Functions.h"

#include "BouncingBall.hpp"


std::string fmu4cpp::model_identifier() {
    return "bouncing_ball";
}


TEST_CASE("BouncingBall_test") {

    BouncingBall model({});
    const auto guid = model.guid();

    auto startHeight = model.get_real_variable("height")->get();

    auto c = fmi2Instantiate("bouncing_ball", fmi2CoSimulation, guid.c_str(), "", nullptr, false, false);
    REQUIRE(c);

    REQUIRE(fmi2SetupExperiment(c, fmi2False, 0, 0, fmi2False, 0) == fmi2OK);
    REQUIRE(fmi2EnterInitializationMode(c) == fmi2OK);
    REQUIRE(fmi2ExitInitializationMode(c) == fmi2OK);

    double t = 0;
    double dt = 0.1;

    REQUIRE(fmi2DoStep(c, t, dt, true) == fmi2OK);
    t += dt;

    const auto heightVr = model.get_real_variable("height")->value_reference();

    double height;
    REQUIRE(fmi2GetReal(c, &heightVr, 1, &height) == fmi2OK);
    REQUIRE(height < 10.0);

    double heightState = height;

    fmi2FMUstate state;
    REQUIRE(fmi2GetFMUstate(c, &state) == fmi2OK);

    REQUIRE(fmi2DoStep(c, t, dt, true) == fmi2OK);
    t += dt;

    REQUIRE(fmi2GetReal(c, &heightVr, 1, &height) == fmi2OK);
    CHECK(height != Catch::Approx(heightState));

    REQUIRE(fmi2SetFMUstate(c, state) == fmi2OK);
    REQUIRE(fmi2GetReal(c, &heightVr, 1, &height) == fmi2OK);
    CHECK(height == Catch::Approx(heightState));


    size_t serializedSize;
    fmi2SerializedFMUstateSize(c, state, &serializedSize);
    CHECK(serializedSize > 0);

    std::vector<char> serializedData(serializedSize);
    fmi2SerializeFMUstate(c, state, serializedData.data(), serializedSize);
    fmi2DeSerializeFMUstate(c, serializedData.data(), serializedSize, &state);

    REQUIRE(fmi2DoStep(c, t, dt, true) == fmi2OK);

    REQUIRE(fmi2GetReal(c, &heightVr, 1, &height) == fmi2OK);
    CHECK(height != Catch::Approx(heightState));

    REQUIRE(fmi2SetFMUstate(c, state) == fmi2OK);
    REQUIRE(fmi2GetReal(c, &heightVr, 1, &height) == fmi2OK);
    CHECK(height == Catch::Approx(heightState));


    REQUIRE(fmi2FreeFMUstate(c, &state) == fmi2OK);
    REQUIRE(state == nullptr);

    REQUIRE(fmi2GetReal(c, &heightVr, 1, &height) == fmi2OK);
    CHECK(height < startHeight );
    fmi2Reset(c);
    REQUIRE(fmi2GetReal(c, &heightVr, 1, &height) == fmi2OK);
    CHECK(height == Catch::Approx(startHeight));

    fmi2FreeInstance(c);
}
