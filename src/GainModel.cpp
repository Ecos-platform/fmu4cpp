#include <fmu4cpp/fmu_base.hpp>

using namespace fmu4cpp;

class GainModel : public fmu_base {
public:
    FMU4CPP_CTOR(GainModel) {
        register_real("u", &u)
            .setCausality(causality_t::INPUT);

        register_real("y", &y)
            .setCausality(causality_t::OUTPUT);

        register_real("k", &k)
            .setCausality(causality_t::PARAMETER)
            .setVariability(variability_t::FIXED);

        reset();
    }

    bool do_step(double dt) override {
        (void)dt;
        y = k * u;
        return true;
    }

    void reset() override {
        u = 0.0;
        y = 0.0;
        k = 2.0;
    }

private:
    double u{};
    double y{};
    double k{};
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "GainModel";
    info.author = "Kevin";
    info.description = "Simple gain FMU made with fmu4cpp";
    info.version = "0.1.0";
    return info;
}

FMU4CPP_INSTANTIATE(GainModel);