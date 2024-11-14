
#include <fmu4cpp/fmu_base.hpp>

using namespace fmu4cpp;


class BouncingBall : public fmu_base {

public:
    BouncingBall(const std::string &instanceName, const std::filesystem::path &resources)
        : fmu_base(instanceName, resources) {

        register_variable(
                real(
                        "height", [this] { return height_; })
                        .setCausality(causality_t::OUTPUT)
                        .setVariability(variability_t::CONTINUOUS)
                        .setInitial(initial_t::EXACT));

        register_variable(
                real(
                        "velocity", [this] { return velocity_; })
                        .setCausality(causality_t::LOCAL)
                        .setVariability(variability_t::CONTINUOUS));

        register_variable(
                real(
                        "gravity", [this] { return gravity_; },
                        [this](const auto &input) { gravity_ = input; })
                        .setCausality(causality_t::PARAMETER)
                        .setVariability(variability_t::FIXED));

        register_variable(
                real(
                        "bounceFactor",
                        [this] { return bounceFactor_; },
                        [this](const auto &input) { bounceFactor_ = input; })
                        .setCausality(causality_t::PARAMETER)
                        .setVariability(variability_t::FIXED));


        BouncingBall::reset();
    }

    bool do_step(double currentTime, double dt) override {
        // Update velocity with gravity
        velocity_ += gravity_ * dt;
        // Update height with current velocity
        height_ += velocity_ * dt;

        // Check for bounce
        if (height_ <= 0.0f) {
            height_ = 0.0f;                        // Reset height to ground level
            velocity_ = -velocity_ * bounceFactor_;// Reverse velocity and apply bounce factor
        }

        return true;
    }

    void reset() override {
        height_ = 10;
        velocity_ = 0;
        gravity_ = -9.81f;
        bounceFactor_ = 0.6f;
    }

private:
    double height_;      // Current height of the ball
    double velocity_;    // Current velocity of the ball
    double gravity_;     // Acceleration due to gravity
    double bounceFactor_;// Factor to reduce velocity on bounce
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "BouncingBall";
    info.description = "A bouncing ball model";
    info.modelIdentifier = FMU4CPP_MODEL_IDENTIFIER;
    return info;
}

std::unique_ptr<fmu_base> fmu4cpp::createInstance(const std::string &instanceName, const std::filesystem::path &fmuResourceLocation) {
    return std::make_unique<BouncingBall>(instanceName, fmuResourceLocation);
}
