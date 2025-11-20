
#ifndef FMU4CPP_TEMPLATE_BOUNCINBALL_HPP
#define FMU4CPP_TEMPLATE_BOUNCINBALL_HPP


#include <fmu4cpp/fmu_base.hpp>

#include <cstring>
#include <iomanip>
#include <sstream>

using namespace fmu4cpp;


class BouncingBall : public fmu_base {

public:
    FMU4CPP_CTOR(BouncingBall) {

        register_real(
                "height", &state_.height_)
                .setCausality(causality_t::OUTPUT)
                .setVariability(variability_t::CONTINUOUS)
                .setInitial(initial_t::EXACT)
                .setDescription("Current height of the ball");

        register_real(
                "velocity", &state_.velocity_)
                .setCausality(causality_t::LOCAL)
                .setVariability(variability_t::CONTINUOUS)
                .setDescription("Current velocity of the ball");

        register_real(
                "gravity", &state_.gravity_)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED)
                .setDescription("Acceleration due to gravity");

        register_real(
                "bounceFactor", &state_.bounceFactor_)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED)
                .setDescription("Factor to reduce velocity on bounce");

        register_state(&BouncingBall::state_);
    }

    bool do_step(double dt) override {
        // Update velocity with gravity
        state_.velocity_ += state_.gravity_ * dt;
        // Update height with current velocity
        state_.height_ += state_.velocity_ * dt;

        // Check for bounce
        if (state_.height_ <= 0.0f) {
            state_.height_ = 0.0f;                                      // Reset height to ground level
            state_.velocity_ = -state_.velocity_ * state_.bounceFactor_;// Reverse velocity and apply bounce factor
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2)
            << "Current height: " << state_.height_
            << ", Current velocity: " << state_.velocity_;

        debugLog(fmiOK, oss.str());

        return true;
    }

private:
    struct State {
        double height_ = 10;       // Current height of the ball
        double velocity_ = 0;      // Current velocity of the ball
        double gravity_ = -9.81;   // Acceleration due to gravity
        double bounceFactor_ = 0.6;// Factor to reduce velocity on bounce
    };

    State state_;
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "BouncingBall";
    info.description = "A bouncing ball model";
    info.canGetAndSetFMUstate = true;
    info.canSerializeFMUstate = true;
    return info;
}

FMU4CPP_INSTANTIATE(BouncingBall);


#endif//FMU4CPP_TEMPLATE_BOUNCINBALL_HPP
