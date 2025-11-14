

#include <fmu4cpp/fmu_base.hpp>

#include <fmt/format.h>

class ExternalLib : public fmu4cpp::fmu_base {

public:
    FMU4CPP_CTOR(ExternalLib) {

        register_variable(
                integer("counter", &counter)
                        .setCausality(fmu4cpp::causality_t::OUTPUT)
                        .setVariability(fmu4cpp::variability_t::DISCRETE));

        ExternalLib::reset();
    }

    bool do_step(double dt) override {

        log(fmiOK, fmt::format("Hello from External Library! counter={} dt={}", ++counter, dt));

        return true;
    }

    void reset() override {
        counter = 0;
    }

private:
    int counter{};
};

fmu4cpp::model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "ExternalLib";
    info.description = "Example of using a external library";
    return info;
}

FMU4CPP_INSTANTIATE(ExternalLib);
