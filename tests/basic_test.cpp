
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <fmu4cpp/fmu_base.hpp>

class Model : public fmu4cpp::fmu_base {

public:
    Model(const std::string &instanceName, const std::string &resources)
        : fmu4cpp::fmu_base(instanceName, resources) {}

    bool do_step(double currentTime, double dt) override {
        return true;
    }
};

fmu4cpp::model_info fmu4cpp::get_model_info() {
    fmu4cpp::model_info m;
    m.modelIdentifier = FMU4CPP_MODEL_IDENTIFIER;
    return m;
}

std::unique_ptr<fmu4cpp::fmu_base> fmu4cpp::createInstance(const std::string &instanceName, const std::string &fmuResourceLocation) {
    return std::make_unique<Model>(instanceName, fmuResourceLocation);
}

TEST_CASE("basic") {
}