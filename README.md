# FMU4cpp

FMU4cpp is a GitHub template repository that allows you to easily create cross-platform FMUs 
compatible with [FMI 2.0](https://fmi-standard.org/downloads/) & [FMI 3.0](https://fmi-standard.org/docs/3.0/) for Co-simulation using CMake and C++.

The framework generates the required `modelDescription.xml` and further packages 
the necessary content into a ready-to-use FMU archive.

### How do I get started?

1. Clone this repository using the "Use this template" button.
2. Browse the example models from the [examples](https://github.com/Ecos-platform/fmu4cpp/tree/examples) branch.
3. Implement your own model by inheriting from the provided `fmu_base` abstract class.
4. Create an FMU target by using the provided `generate_fmu` CMake function.
5. Build. Models for your platform are located in a `models` folder within the build folder.
6. Upload your changes to GitHub to trigger cross-compilation and validation against [fmusim](https://github.com/modelica/Reference-FMUs).
7. Download cross-compiled FMUs from the Actions tab.

#### Alternate workflow using CMake FetchContent
You can also use FMU4cpp as a CMake FetchContent dependency in a pre-existing CMake project.

```cmake
include(FetchContent)
FetchContent_Declare(
    fmu4cpp
    GIT_REPOSITORY https://github.com/Ecos-platform/fmu4cpp.git
    GIT_TAG git_tag_commit_or_branch_to_use
)
FetchContent_MakeAvailable(fmu4cpp)
```

Then, you can create your own FMU by inheriting from `fmu4cpp::fmu_base` and using the `generate_fmu` function.

### Example (BouncingBall)

```cpp
#include <fmu4cpp/fmu_base.hpp>

using namespace fmu4cpp;


class BouncingBall : public fmu_base {

public:
    FMU4CPP_CTOR(BouncingBall) {

        register_real(
                "height", &height)
                .setCausality(causality_t::OUTPUT)
                .setVariability(variability_t::CONTINUOUS))
                .setInitial(initial_t::EXACT);

        register_real(
                "velocity", &velocity)
                .setCausality(causality_t::LOCAL)
                .setVariability(variability_t::CONTINUOUS);

        register_real(
                "gravity", &gravity)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED);

        register_real(
                "bounceFactor", &bounceFactor)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED);


        BouncingBall::reset();
    }

    bool do_step(double dt) override {
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
    double height{};      // Current height of the ball
    double velocity{};    // Current velocity of the ball
    double gravity{};     // Acceleration due to gravity
    double bounceFactor{};// Factor to reduce velocity on bounce
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "BouncingBall";
    info.description = "A bouncing ball model";
    return info;
}

FMU4CPP_INSTANTIATE(BouncingBall);

```


#### Cross-compilation

Cross-compilation (64-bit linux/windows) occurs automatically when you push your changes to GitHub.


Such easy, such wow.


### Requirements
* C++17 compiler
* CMake >= 3.15
