
#include <fmu4cpp/fmu_base.hpp>

#include <iomanip>
#include <sstream>

using namespace fmu4cpp;


class BouncingBall : public fmu_base {

public:
    explicit BouncingBall(const fmu_data &data) : fmu_base(data) {

        register_variable(
                real(
                        "height", &height_)
                        .setCausality(causality_t::OUTPUT)
                        .setVariability(variability_t::CONTINUOUS)
                        .setInitial(initial_t::EXACT));

        register_variable(
                real(
                        "velocity", &velocity_)
                        .setCausality(causality_t::LOCAL)
                        .setVariability(variability_t::CONTINUOUS));

        register_variable(
                real(
                        "gravity", &gravity_)
                        .setCausality(causality_t::PARAMETER)
                        .setVariability(variability_t::FIXED));

        register_variable(
                real(
                        "bounceFactor", &bounceFactor_)
                        .setCausality(causality_t::PARAMETER)
                        .setVariability(variability_t::FIXED));

        BouncingBall::reset();
    }

    bool do_step(double dt) override {
        // Update velocity with gravity
        velocity_ += gravity_ * dt;
        // Update height with current velocity
        height_ += velocity_ * dt;

        // Check for bounce
        if (height_ <= 0.0f) {
            height_ = 0.0f;                        // Reset height to ground level
            velocity_ = -velocity_ * bounceFactor_;// Reverse velocity and apply bounce factor
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2)
            << "Current height: " << height_
            << ", Current velocity: " << velocity_;

        log(fmiOK, oss.str());

        return true;
    }

    void reset() override {
        height_ = 10;
        velocity_ = 0;
        gravity_ = -9.81f;
        bounceFactor_ = 0.6f;
    }

private:
    double height_{};      // Current height of the ball
    double velocity_{};    // Current velocity of the ball
    double gravity_{};     // Acceleration due to gravity
    double bounceFactor_{};// Factor to reduce velocity on bounce
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "BouncingBall";
    info.description = "A bouncing ball model";
    info.modelIdentifier = FMU4CPP_MODEL_IDENTIFIER;
    return info;
}

FMU4CPP_INSTANTIATE(BouncingBall);
