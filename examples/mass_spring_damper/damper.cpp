
#include <fmu4cpp/fmu_base.hpp>

using namespace fmu4cpp;

class Damper : public fmu_base {

public:
    FMU4CPP_CTOR(Damper) {

        register_real("velocity", &state_.v_)
                .setCausality(causality_t::INPUT)
                .setVariability(variability_t::CONTINUOUS)
                .setDescription("Mass velocity");

        register_real("damping", &state_.c_)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED)
                .setDescription("Damping coefficient");

        register_real("force", &state_.F_)
                .setCausality(causality_t::OUTPUT)
                .setVariability(variability_t::CONTINUOUS)
                .setDescription("Damper force");

        register_state(&Damper::state_);
    }

    bool do_step(double) override {
        state_.F_ = -state_.c_ * state_.v_;
        return true;
    }

private:
    struct State {
        double v_ = 0.0;
        double c_ = 1.0;
        double F_ = 0.0;
    };

    State state_;
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Damper";
    info.description = "A damper model";
    info.canGetAndSetFMUstate = true;
    info.canSerializeFMUstate = true;
    return info;
}

FMU4CPP_INSTANTIATE(Damper);
