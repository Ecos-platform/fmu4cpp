
#include <fmu4cpp/fmu_base.hpp>

#include <cmath>
#include <numbers>

using namespace fmu4cpp;


class SimplePendulum : public fmu_base {
public:
    SimplePendulum(const std::string &instanceName, const std::string &resources)
        : fmu_base(instanceName, resources) {

        register_variable(real("angle",
                               [this] { return angle_; })
                                  .setCausality(causality_t::OUTPUT)
                                  .setVariability(variability_t::CONTINUOUS));

        register_variable(real("angularVelocity",
                               [this] { return angularVelocity_; })
                                  .setCausality(causality_t::LOCAL)
                                  .setVariability(variability_t::CONTINUOUS));
        register_variable(real(
                                  "gravity",
                                  [this] { return gravity_; },
                                  [this](double input) { gravity_ = input; })
                                  .setCausality(causality_t::PARAMETER)
                                  .setVariability(variability_t::FIXED));
        register_variable(real(
                                  "length",
                                  [this] { return length_; },
                                  [this](double input) { length_ = input; })
                                  .setCausality(causality_t::PARAMETER)
                                  .setVariability(variability_t::FIXED));
        register_variable(real(
                                  "damping",
                                  [this] { return damping_; },
                                  [this](double input) { damping_ = input; })
                                  .setCausality(causality_t::PARAMETER)
                                  .setVariability(variability_t::FIXED));

        SimplePendulum::reset();
    }

    bool do_step(double currentTime, double dt) override {
        angularVelocity_ += (-gravity_ / length_) * std::sin(angle_) * dt - damping_ * angularVelocity_ * dt;
        angle_ += angularVelocity_ * dt;
        return true;
    }

    void reset() override {
        angle_ = std::numbers::pi / 4;// 45 degrees
        angularVelocity_ = 0;
        gravity_ = -9.81;
        length_ = 1.0;
        damping_ = 0.1;
    }

private:
    double angle_;          // Current angle of the pendulum
    double angularVelocity_;// Current angular velocity
    double gravity_;        // Acceleration due to gravity
    double length_;         // Length of the pendulum
    double damping_;        // Damping factor
};


model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "SimplePendulum";
    info.description = "A simple pendulum model";
    info.modelIdentifier = FMU4CPP_MODEL_IDENTIFIER;
    return info;
}

std::unique_ptr<fmu_base> fmu4cpp::createInstance(const std::string &instanceName, const std::string &fmuResourceLocation) {
    return std::make_unique<SimplePendulum>(instanceName, fmuResourceLocation);
}
