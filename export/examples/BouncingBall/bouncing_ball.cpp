
#include <fmu4cpp/fmu_base.hpp>
#include <utility>


using namespace fmu4cpp;

class BouncingBall : public fmu_base {

public:
    BouncingBall(const std::string &instanceName, const std::string &resources)
        : fmu_base(instanceName, resources) {

        register_variable(
                real(
                        "height", [this] { return height; })
                        .setCausality(causality_t::OUTPUT)
                        .setVariability(variability_t::CONTINUOUS));

        register_variable(
                real(
                        "velocity", [this] { return velocity; })
                        .setCausality(causality_t::CALCULATED_PARAMETER)
                        .setVariability(variability_t::TUNABLE));

        register_variable(
                real(
                        "gravity", [this] { return gravity; })
                        .setCausality(causality_t::PARAMETER)
                        .setVariability(variability_t::TUNABLE));

        register_variable(
                real(
                        "bounceFactor", [this] { return bounceFactor; })
                        .setCausality(causality_t::PARAMETER)
                        .setVariability(variability_t::TUNABLE));


        BouncingBall::reset();
    }

    bool do_step(double currentTime, double dt) override {
        // Update velocity with gravity
        velocity += gravity * dt;
        // Update height with current velocity
        height += velocity * dt;

        // Check for bounce
        if (height <= 0.0f) {
            height = 0.0f;                      // Reset height to ground level
            velocity = -velocity * bounceFactor;// Reverse velocity and apply bounce factor
        }

        return true;
    }

    void reset() override {
        height = 10;
        velocity = 0;
        gravity = -9.81f;
        bounceFactor = 0.6f;
    }

private:
    double height;      // Current height of the ball
    double velocity;    // Current velocity of the ball
    double gravity;     // Acceleration due to gravity
    double bounceFactor;// Factor to reduce velocity on bounce
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "BouncingBall";
    info.description = "A bouncing ball model";
    info.modelIdentifier = FMU4CPP_MODEL_IDENTIFIER;
    return info;
}

std::unique_ptr<fmu_base> fmu4cpp::createInstance(const std::string &instanceName, const std::string &fmuResourceLocation) {
    return std::make_unique<BouncingBall>(instanceName, fmuResourceLocation);
}
