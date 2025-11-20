
#include <fmu4cpp/fmu_base.hpp>

#include <cmath>

using namespace fmu4cpp;

constexpr double pi = 3.14159265358979323846;

class SimplePendulum : public fmu_base {

public:
    FMU4CPP_CTOR(SimplePendulum) {

        register_real("angle", &state_.angle_)
                .setCausality(causality_t::OUTPUT)
                .setVariability(variability_t::CONTINUOUS);

        register_real("angularVelocity", &state_.angularVelocity_)
                .setCausality(causality_t::LOCAL)
                .setVariability(variability_t::CONTINUOUS);
        register_real(
                "gravity", &state_.gravity_)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED);
        register_real(
                "length", &state_.length_)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED);
        register_real(
                "damping", &state_.damping_)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED)
                .addAnnotation("<Tool name=\"fmu4cpp\">\n\t<documentation>\"Example of tool specific variable annotation\"</documentation>\n</Tool>");

        register_state(&SimplePendulum::state_);
    }

    bool do_step(double dt) override {
        state_.angularVelocity_ += (-state_.gravity_ / state_.length_) * std::sin(state_.angle_) * dt - state_.damping_ * state_.angularVelocity_ * dt;
        state_.angle_ += state_.angularVelocity_ * dt;
        return true;
    }

private:
    struct State {
        double angle_ = pi / 4;     // Current angle of the pendulum
        double angularVelocity_ = 0;// Current angular velocity
        double gravity_ = -9.81;    // Acceleration due to gravity
        double length_ = 1;         // Length of the pendulum
        double damping_ = 0.5;      // Damping factor
    };

    State state_;
};


model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "SimplePendulum";
    info.description = "A simple pendulum model";
    info.canGetAndSetFMUstate = true;
    info.canSerializeFMUstate = true;
    info.defaultExperiment = {0.0, 10};
    info.vendorAnnotations = {"<Tool name=\"fmu4cpp\">\n\t<documentation>\"Example of tool specific annotation data\"</documentation>\n</Tool>"};
    return info;
}

FMU4CPP_INSTANTIATE(SimplePendulum);
