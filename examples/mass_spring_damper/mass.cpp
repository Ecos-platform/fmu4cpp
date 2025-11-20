
#include <fmu4cpp/fmu_base.hpp>

using namespace fmu4cpp;

class Mass : public fmu_base {

public:
    FMU4CPP_CTOR(Mass) {

        register_real("position", &state_.x_)
                .setCausality(causality_t::OUTPUT)
                .setVariability(variability_t::CONTINUOUS)
                .setInitial(initial_t::EXACT)
                .setDescription("Mass position");

        register_real("velocity", &state_.v_)
                .setCausality(causality_t::OUTPUT)
                .setVariability(variability_t::CONTINUOUS)
                .setDescription("Mass velocity");

        register_real("mass", &state_.m_)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED)
                .setDescription("Mass value");

        register_real("force_spring", &state_.F_spring_)
                .setCausality(causality_t::INPUT)
                .setVariability(variability_t::CONTINUOUS)
                .setDescription("Force from spring");

        register_real("force_damper", &state_.F_damper_)
                .setCausality(causality_t::INPUT)
                .setVariability(variability_t::CONTINUOUS)
                .setDescription("Force from damper");

        register_real("force_external", &state_.F_external_)
                .setCausality(causality_t::INPUT)
                .setVariability(variability_t::CONTINUOUS)
                .setDescription("External applied force");

        register_state(&Mass::state_);
    }

    bool do_step(double dt) override {

        double F = state_.F_spring_ + state_.F_damper_ + state_.F_external_;
        double a = F / state_.m_;

        state_.v_ += a * dt;
        state_.x_ += state_.v_ * dt;

        return true;
    }

private:
    struct State {
        double x_ = 0.5;
        double v_ = 0.0;
        double m_ = 1.0;

        double F_spring_ = 0.0;
        double F_damper_ = 0.0;
        double F_external_ = 0.0;
    };

    State state_;
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Mass";
    info.description = "A mass model";
    info.canGetAndSetFMUstate = true;
    info.canSerializeFMUstate = true;
    return info;
}

FMU4CPP_INSTANTIATE(Mass);
