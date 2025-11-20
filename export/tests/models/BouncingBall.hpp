
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
                .setInitial(initial_t::EXACT);

        register_real(
                "velocity", &state_.velocity_)
                .setCausality(causality_t::LOCAL)
                .setVariability(variability_t::CONTINUOUS);

        register_real(
                "gravity", &state_.gravity_)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED);

        register_real(
                "bounceFactor", &state_.bounceFactor_)
                .setCausality(causality_t::PARAMETER)
                .setVariability(variability_t::FIXED);

        BouncingBall::reset();
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

        log(fmiOK, oss.str());

        return true;
    }

    void *getFMUState() override {
        auto statePtr = new State(state_);
        return statePtr;
    }

    void setFmuState(void *state) override {
        state_.setFromPtr(state);
    }

    void freeFmuState(void **state) override {
        delete static_cast<State *>(*state);
        *state = nullptr;
    }

    void serializedFMUStateSize(void *state, size_t &size) override {
        size = sizeof(State);
    }

    void serializeFMUState(void *state, std::vector<uint8_t> &serializedState) override {
        const State *statePtr = static_cast<State *>(state);
        serializedState.resize(sizeof(State));
        std::memcpy(serializedState.data(), statePtr, sizeof(State));
    }

    void deserializeFMUState(const std::vector<uint8_t> &serializedState, void **state) override {
        auto statePtr = new State();
        std::memcpy(statePtr, serializedState.data(), sizeof(State));
        *state = statePtr;
    }

    void reset() override {
        state_.reset();
    }

private:
    struct State {
        double height_{};      // Current height of the ball
        double velocity_{};    // Current velocity of the ball
        double gravity_{};     // Acceleration due to gravity
        double bounceFactor_{};// Factor to reduce velocity on bounce

        void setFromPtr(const void *ptr) {
            const auto *src = static_cast<const State *>(ptr);
            *this = *src;
        }

        void reset() {
            height_ = 10;
            velocity_ = 0;
            gravity_ = -9.81f;
            bounceFactor_ = 0.6f;
        }
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
