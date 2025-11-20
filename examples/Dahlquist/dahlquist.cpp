
#include "fmu4cpp/fmu_base.hpp"

#include <cmath>

class Dahlquist : public fmu4cpp::fmu_base {

public:
    FMU4CPP_CTOR(Dahlquist) {

        register_real("x", &x)
                .setCausality(fmu4cpp::causality_t::OUTPUT)
                .setVariability(fmu4cpp::variability_t::CONTINUOUS)
                .setInitial(fmu4cpp::initial_t::EXACT);

        register_real("dx", &dx)
                .setCausality(fmu4cpp::causality_t::LOCAL)
                .setVariability(fmu4cpp::variability_t::CONTINUOUS);

        register_real("k", &k)
                .setCausality(fmu4cpp::causality_t::PARAMETER)
                .setVariability(fmu4cpp::variability_t::FIXED);

        Dahlquist::reset();
    }

    bool do_step(double dt) override {

        const double lambda = k;
        // exact Dahlquist step: x_{n+1} = exp(lambda * dt) * x_n
        double x_new = std::exp(lambda * dt) * x;
        // derivative at the new state
        dx = lambda * x_new;
        x = x_new;

        return true;
    }


    void reset() override {
        x = 1.0;
        dx = 0.0;
        k = -1.0;
    }

private:
    double x{};
    double dx{};
    double k{};
};

fmu4cpp::model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Dahlquist";
    info.description = "This model implements the Dahlquist test equation";
    info.defaultExperiment = {0, 10, 0.1};
    return info;
}

FMU4CPP_INSTANTIATE(Dahlquist);
