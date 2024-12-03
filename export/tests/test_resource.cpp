
#include <catch2/catch_test_macros.hpp>

#include <fmu4cpp/fmu_base.hpp>
#include <fstream>

class Model : public fmu4cpp::fmu_base {

public:
    Model(const std::string &instanceName, const std::filesystem::path &resources)
        : fmu_base(instanceName, resources) {

        std::ifstream file(resources.string() + "/data.txt");

        std::getline(file, data_);

        register_variable(string("data", &data_)
                                  .setCausality(fmu4cpp::causality_t::OUTPUT));

        Model::reset();
    }

    bool do_step(double currentTime, double dt) override {

        return true;
    }

    void reset() override {
        // do nothing
    }

private:
    std::string data_;
};

fmu4cpp::model_info fmu4cpp::get_model_info() {
    model_info m;
    m.modelIdentifier = FMU4CPP_MODEL_IDENTIFIER;
    return m;
}

std::unique_ptr<fmu4cpp::fmu_base> fmu4cpp::createInstance(const std::string &instanceName, const std::filesystem::path &fmuResourceLocation) {
    return std::make_unique<Model>(instanceName, fmuResourceLocation);
}

TEST_CASE("model with resource") {

    std::string expected{"Hello resource!"};

    const auto instance = fmu4cpp::createInstance("myInstance", TEST_CASE_RESOURCE_LOCATION);

    auto strVar = instance->get_string_variable("data");
    REQUIRE(strVar);

    double t = 0;
    double dt = 0.1;
    instance->setup_experiment(t, {}, {});
    instance->enter_initialisation_mode();
    instance->exit_initialisation_mode();

    while (t < 10) {
        instance->do_step(t, dt);

        REQUIRE(strVar->get() == expected);

        t += dt;
    }

    instance->reset();

    REQUIRE(strVar->get() == expected);

    instance->terminate();
}
