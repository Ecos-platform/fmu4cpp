
#include <fmu4cpp/fmu_base.hpp>

using namespace fmu4cpp;

class Spring : public fmu_base {

public:
    FMU4CPP_CTOR(Spring) {

        register_real("position", &state_.x_)
                .setCausality(causality_t::INPUT)
                .setVariability(variability_t::CONTINUOUS)
                .setDescription("Mass position");

        register_real("rest_position", &state_.x0_)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED)
                .setDescription("Rest position");

        register_real("stiffness", &state_.k_)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED)
                .setDescription("Spring constant");

        register_real("force", &state_.F_)
                .setCausality(causality_t::OUTPUT)
                .setVariability(variability_t::CONTINUOUS)
                .setDescription("Spring force");

        register_state(&Spring::state_);
    }

    bool do_step(double dt) override {
        state_.F_ = -state_.k_ * (state_.x_ - state_.x0_);
        return true;
    }

private:
    struct State {
        double x_ = 0.0;
        double x0_ = 0.0;
        double k_ = 10.0;
        double F_ = 0.0;
    };

    State state_;
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Spring";
    info.description = "A spring model";
    info.canGetAndSetFMUstate = true;
    info.canSerializeFMUstate = true;
    return info;
}

FMU4CPP_INSTANTIATE(Spring);
