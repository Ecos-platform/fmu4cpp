
#ifndef IDENTITY_HPP
#define IDENTITY_HPP

#include <fmu4cpp/fmu_base.hpp>

using namespace fmu4cpp;

class Model : public fmu_base {

public:
    FMU4CPP_CTOR(Model) {

        register_variable(integer(
                                  "integerIn", &state_.integer_)
                                  .setCausality(causality_t::INPUT)
                                  .setVariability(variability_t::DISCRETE));
        register_variable(
                real(
                        "realIn", &state_.real_)
                        .setCausality(causality_t::INPUT)
                        .setVariability(variability_t::DISCRETE));

        register_variable(boolean(
                                  "booleanIn", &state_.boolean_)
                                  .setCausality(causality_t::INPUT)
                                  .setVariability(variability_t::DISCRETE));

        register_variable(string(
                                  "stringIn", &state_.string_)
                                  .setCausality(causality_t::INPUT)
                                  .setVariability(variability_t::DISCRETE));

        register_variable(integer("integerOut", &state_.integer_)
                                  .setCausality(causality_t::OUTPUT)
                                  .setVariability(variability_t::DISCRETE)
                                  .setInitial(initial_t::CALCULATED)
                                  .setDependencies({"integerIn"}));

        register_variable(real("realOut", &state_.real_)
                                  .setCausality(causality_t::OUTPUT)
                                  .setVariability(variability_t::DISCRETE)
                                  .setInitial(initial_t::CALCULATED)
                                  .setDependencies({"realIn"}));

        register_variable(boolean("booleanOut", &state_.boolean_)
                                  .setCausality(causality_t::OUTPUT)
                                  .setVariability(variability_t::DISCRETE)
                                  .setInitial(initial_t::CALCULATED)
                                  .setDependencies({"booleanIn"}));

        register_variable(string("stringOut", &state_.string_)
                                  .setCausality(causality_t::OUTPUT)
                                  .setVariability(variability_t::DISCRETE)
                                  .setInitial(initial_t::CALCULATED)
                                  .setDependencies({"stringIn"}));

        Model::reset();
    }

    bool do_step(double dt) override {
        log(fmiOK, "hello@ " + std::to_string(currentTime()));
        return true;
    }

    void *getFMUState() override {
        auto* state = new State(state_);
        return state;
    }

    void setFmuState(void *state) override {
        state_ = *static_cast<State *>(state);
    }

    void freeFmuState(void **state) override {
        delete static_cast<State *>(*state);
        *state = nullptr;
    }

    void reset() override {
       state_.reset();
    }

private:

    struct State {
        int integer_{};
        double real_{};
        bool boolean_{};
        std::string string_{};

        void reset() {
            integer_ = 0;
            real_ = 0;
            boolean_ = false;
            string_ = "empty";
        }
    };

    State state_;
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Identity";
    info.description = "A simple feed-trough model";
    info.defaultExperiment = {0.0, 10};
    info.canGetAndSetFMUstate = true;

    return info;
}

FMU4CPP_INSTANTIATE(Model);// Entry point for FMI instantiate function.


#endif //IDENTITY_HPP
