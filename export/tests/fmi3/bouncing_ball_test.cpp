
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "fmi3/fmi3Functions.h"

#include "BouncingBall.hpp"


std::string fmu4cpp::model_identifier() {
    return "bouncing_ball";
}


TEST_CASE("BouncingBall_test") {

    BouncingBall model({});
    const auto guid = model.guid();

    auto startHeight = model.get_real_variable("height")->get();

    auto c = fmi3InstantiateCoSimulation("bouncing_ball", guid.c_str(), "", false, false, false, false, nullptr, 0, nullptr, nullptr, nullptr);
    REQUIRE(c);

    REQUIRE(fmi3EnterInitializationMode(c, false, 0, 0, false, 0) == fmi3OK);
    REQUIRE(fmi3ExitInitializationMode(c) == fmi3OK);

    double t = 0;
    double dt = 0.1;

    bool eventhandlingNeeded;
    bool terminateSimulation;
    bool earlyReturn;
    double lastSucessfulTime;
    REQUIRE(fmi3DoStep(c, t, dt, true, &eventhandlingNeeded, &terminateSimulation, &earlyReturn, &lastSucessfulTime) == fmi3OK);
    t += dt;

    const auto heightVr = model.get_real_variable("height")->value_reference();

    double height;
    REQUIRE(fmi3GetFloat64(c, &heightVr, 1, &height, 1) == fmi3OK);
    REQUIRE(height < 10.0);

    double heightState = height;

    fmi3FMUState state;
    REQUIRE(fmi3GetFMUState(c, &state) == fmi3OK);

    REQUIRE(fmi3DoStep(c, t, dt, true, &eventhandlingNeeded, &terminateSimulation, &earlyReturn, &lastSucessfulTime) == fmi3OK);
    t += dt;

    REQUIRE(fmi3GetFloat64(c, &heightVr, 1, &height, 1) == fmi3OK);
    CHECK(height != Catch::Approx(heightState));

    REQUIRE(fmi3SetFMUState(c, state) == fmi3OK);
    REQUIRE(fmi3GetFloat64(c, &heightVr, 1, &height, 1) == fmi3OK);
    CHECK(height == Catch::Approx(heightState));


    size_t serializedSize;
    fmi3SerializedFMUStateSize(c, state, &serializedSize);
    CHECK(serializedSize > 0);

    std::vector<uint8_t> serializedData(serializedSize);
    fmi3SerializeFMUState(c, state, serializedData.data(), serializedSize);
    fmi3DeserializeFMUState(c, serializedData.data(), serializedSize, &state);

    REQUIRE(fmi3DoStep(c, t, dt, true, &eventhandlingNeeded, &terminateSimulation, &earlyReturn, &lastSucessfulTime) == fmi3OK);

    REQUIRE(fmi3GetFloat64(c, &heightVr, 1, &height, 1) == fmi3OK);
    CHECK(height != Catch::Approx(heightState));

    REQUIRE(fmi3SetFMUState(c, state) == fmi3OK);
    REQUIRE(fmi3GetFloat64(c, &heightVr, 1, &height, 1) == fmi3OK);
    CHECK(height == Catch::Approx(heightState));


    REQUIRE(fmi3FreeFMUState(c, &state) == fmi3OK);
    REQUIRE(state == nullptr);


    REQUIRE(fmi3GetFloat64(c, &heightVr, 1, &height, 1) == fmi3OK);
    CHECK(height < startHeight );
    fmi3Reset(c);
    REQUIRE(fmi3GetFloat64(c, &heightVr, 1, &height, 1) == fmi3OK);
    CHECK(height == Catch::Approx(startHeight));

    fmi3FreeInstance(c);
}
