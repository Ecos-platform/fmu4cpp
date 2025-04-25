
#include <fmu4cpp/fmu_base.hpp>

#include <cmath>

using namespace fmu4cpp;

constexpr double pi = 3.14159265358979323846;

class SimplePendulum : public fmu_base {
public:
    SimplePendulum(const std::string &instanceName, const std::filesystem::path &resources)
        : fmu_base(instanceName, resources) {

        register_variable(real("angle", &angle_)
                                  .setCausality(causality_t::OUTPUT)
                                  .setVariability(variability_t::CONTINUOUS));

        register_variable(real("angularVelocity", &angularVelocity_)
                                  .setCausality(causality_t::LOCAL)
                                  .setVariability(variability_t::CONTINUOUS));
        register_variable(real(
                                  "gravity", &gravity_)
                                  .setCausality(causality_t::PARAMETER)
                                  .setVariability(variability_t::FIXED));
        register_variable(real(
                                  "length", &length_)
                                  .setCausality(causality_t::PARAMETER)
                                  .setVariability(variability_t::FIXED));
        register_variable(real(
                                  "damping", &damping_)
                                  .setCausality(causality_t::PARAMETER)
                                  .setVariability(variability_t::FIXED)
                                  .addAnnotation("<Tool name=\"fmu4cpp\">\n\t<documentation>\"Example of tool specific variable annotation\"</documentation>\n</Tool>"));

        SimplePendulum::reset();
    }

    bool do_step(double currentTime, double dt) override {
        angularVelocity_ += (-gravity_ / length_) * std::sin(angle_) * dt - damping_ * angularVelocity_ * dt;
        angle_ += angularVelocity_ * dt;
        return true;
    }

    void reset() override {
        angle_ = pi / 4;// 45 degrees
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
    info.vendorAnnotations = {"<Tool name=\"fmu4cpp\">\n\t<documentation>\"Example of tool specific annotation data\"</documentation>\n</Tool>"};
    return info;
}

FMU4CPP_INSTANTIATE(SimplePendulum);
