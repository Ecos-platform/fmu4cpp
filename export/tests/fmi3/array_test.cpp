
#include "fmi3/fmi3Functions.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <cstdarg>
#include <iostream>
#include <unordered_set>

#include <fmu4cpp/fmu_base.hpp>

class Model : public fmu4cpp::fmu_base {

public:
    FMU4CPP_CTOR(Model), reals_(4) {

        for (int i = 0; i < reals_.size(); i++) {
            // clang-format off
            register_real("real[" + std::to_string(i) + "]",
                [this, i] { return reals_[i]; },
                [this, i](double val) { reals_[i] = val; })
            .setCausality(fmu4cpp::causality_t::PARAMETER)
            .setVariability(fmu4cpp::variability_t::TUNABLE);
            // clang-format on
        }

        Model::reset();
    }

    bool do_step(double dt) override {
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
    return info;
}

std::string fmu4cpp::model_identifier() {
    return "array";
}

FMU4CPP_INSTANTIATE(Model);


void fmilogger(fmi3InstanceEnvironment, fmi3Status status, fmi3String /*category*/, fmi3String message) {

    std::cout << status << ": " << message << std::endl;
}

TEST_CASE("test_array") {

    Model model({});
    const auto guid = model.guid();

    auto vrs = model.get_value_refs();
    REQUIRE(vrs.size() == 4 + 1);// 4 variables + 1 for time
    std::unordered_set<unsigned int> unique_vrs(vrs.begin(), vrs.end());

    // Check that all are unique
    REQUIRE(unique_vrs.size() == vrs.size());

    auto c = fmi3InstantiateCoSimulation("array", guid.c_str(), "", false, true, false, false, nullptr, 0, nullptr, fmilogger, nullptr);
    REQUIRE(c);

    REQUIRE(fmi3EnterInitializationMode(c, false, 0, 0, false, 0) == fmi3OK);
    REQUIRE(fmi3ExitInitializationMode(c) == fmi3OK);


    std::vector<double> values(4);
    for (int i = 1; i <= 4; i++) {// account for time
        fmi3ValueReference ref = i;
        fmi3GetFloat64(c, &ref, 1, &values[i - 1], 1);
    }
    REQUIRE(values == std::vector<double>{1, 2, 3, 4});

    for (int i = 1; i <= 4; i++) {
        fmi3ValueReference ref = i;
        fmi3Float64 val = 9;
        fmi3SetFloat64(c, &ref, 1, &val, 1);
    }

    for (int i = 1; i <= 4; i++) {
        fmi3ValueReference ref = i;
        fmi3GetFloat64(c, &ref, 1, &values[i - 1], 1);
    }
    REQUIRE(values == std::vector<double>{9, 9, 9, 9});


    REQUIRE(fmi3Terminate(c) == fmi3OK);

    fmi3FreeInstance(c);
}
