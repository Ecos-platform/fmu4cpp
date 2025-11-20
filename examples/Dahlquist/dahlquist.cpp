
#include "fmu4cpp/fmu_base.hpp"

#include <cmath>

class Dahlquist : public fmu4cpp::fmu_base {

public:
    FMU4CPP_CTOR(Dahlquist) {

        register_real("x", & state_.x)
                .setCausality(fmu4cpp::causality_t::OUTPUT)
                .setVariability(fmu4cpp::variability_t::CONTINUOUS)
                .setInitial(fmu4cpp::initial_t::EXACT);

        register_real("dx", &state_.dx)
                .setCausality(fmu4cpp::causality_t::LOCAL)
                .setVariability(fmu4cpp::variability_t::CONTINUOUS);

        register_real("k", &state_.k)
                .setCausality(fmu4cpp::causality_t::PARAMETER)
                .setVariability(fmu4cpp::variability_t::FIXED);

        set_state_helpers(&Dahlquist::state_);
    }

    bool do_step(double dt) override {

        const double lambda = state_.k;
        // exact Dahlquist step: x_{n+1} = exp(lambda * dt) * x_n
        double x_new = std::exp(lambda * dt) * state_.x;
        // derivative at the new state
        state_.dx = lambda * x_new;
        state_.x = x_new;

        return true;
    }

private:
    struct State {
        double x = 1.0;
        double dx = 0.0;
        double k = -1.0;
    };

    State state_;
};

fmu4cpp::model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Dahlquist";
    info.description = "This model implements the Dahlquist test equation";
    info.defaultExperiment = {0, 10, 0.1};
    info.canGetAndSetFMUstate = true;
    info.canSerializeFMUstate = true;
    return info;
}

FMU4CPP_INSTANTIATE(Dahlquist);
