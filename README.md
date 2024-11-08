# FMU4cpp (early prototype)

FMU4cpp is a CMake template repository that allows you to easily create cross-platform FMUs 
compatible with [FMI 2.0](https://fmi-standard.org/downloads/) for Co-simulation using C++.

The framework generates the required `modelDescription.xml` and further packages 
the necessary content into a ready-to-use FMU archive.

### How do I get started?

1. Change the value of the `modelIdentifier` variable in `CMakeLists.txt` to something more appropriate.
2. Edit the content of [model.cpp](src/model.cpp).
3. Build.

An FMU named `<modelIdentifier>.fmu` is now located in a folder `\<modelIdentifier>` within your build folder.

### Example (BouncingBall)

```cpp
#include <fmu4cpp/fmu_base.hpp>

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
                        .setCausality(causality_t::LOCAL)
                        .setVariability(variability_t::CONTINUOUS));

        register_variable(
                real(
                        "gravity", [this] { return gravity; },
                        [this](const auto &input) { gravity = input; })
                        .setCausality(causality_t::PARAMETER)
                        .setVariability(variability_t::FIXED));

        register_variable(
                real(
                        "bounceFactor",
                        [this] { return bounceFactor; },
                        [this](const auto &input) { bounceFactor = input; })
                        .setCausality(causality_t::PARAMETER)
                        .setVariability(variability_t::FIXED));


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

```


#### Cross-compilation

Cross-compilation (64-bit linux/windows) occurs automatically when you push your changes to GitHub. 
Simply rename the produced `model.zip` to `<modelName>.fmu`.


Such easy, such wow.


### Requirements
* C++17 compiler
* CMake >= 3.15
