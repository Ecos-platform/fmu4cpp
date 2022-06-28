//
// FMU4cpp Hello World example
//

#include <fmu4cpp/fmu_base.hpp>
#include <utility>


using namespace fmu4cpp;

class Model : public fmu_base {

public:
    Model(const std::string &instanceName, const std::string &resources)
        : fmu_base(instanceName, resources) {

        register_variable(
                integer(
                        "integerIn", [this] { return integer_; }, [this](int value) { integer_ = value; })
                        .setCausality(causality_t::INPUT)
                        .setVariability(variability_t::DISCRETE));
        register_variable(
                real(
                        "realIn", [this] { return real_; }, [this](double value) { real_ = value; })
                        .setCausality(causality_t::INPUT)
                        .setVariability(variability_t::DISCRETE));
        register_variable(
                boolean(
                        "booleanIn", [this] { return boolean_; }, [this](bool value) { boolean_ = value; })
                        .setCausality(causality_t::INPUT)
                        .setVariability(variability_t::DISCRETE));
        register_variable(
                string(
                        "stringIn", [this] { return string_; }, [this](std::string value) { string_ = std::move(value); })
                        .setCausality(causality_t::INPUT)
                        .setVariability(variability_t::DISCRETE));

        register_variable(
                integer(
                        "integerOut", [this] { return integer_; })
                        .setCausality(causality_t::OUTPUT)
                        .setVariability(variability_t::DISCRETE)
                        .setDependencies({get_int_variable("integerIn")->index()}));
        register_variable(
                real(
                        "realOut", [this] { return real_; })
                        .setCausality(causality_t::OUTPUT)
                        .setVariability(variability_t::DISCRETE)
                        .setDependencies({get_real_variable("realIn")->index()}));
        register_variable(
                boolean(
                        "booleanOut", [this] { return boolean_; })
                        .setCausality(causality_t::OUTPUT)
                        .setVariability(variability_t::DISCRETE)
                        .setDependencies({get_bool_variable("booleanIn")->index()}));
        register_variable(
                string(
                        "stringOut", [this] { return string_; })
                        .setCausality(causality_t::OUTPUT)
                        .setVariability(variability_t::DISCRETE)
                        .setDependencies({get_string_variable("stringIn")->index()}));

        Model::reset();
    }

    bool do_step(double currentTime, double dt) override {
        //log(fmi2OK, "hei");
        return true;
    }

    void reset() override {
        integer_ = 0;
        real_ = 0;
        boolean_ = false;
        string_ = "empty";
    }

private:
    int integer_;
    double real_;
    bool boolean_;
    std::string string_;
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Identity";
    info.description = "A simple feed-trough model";
    info.modelIdentifier = FMU4CPP_MODEL_IDENTIFIER;
    return info;
}

std::unique_ptr<fmu_base> fmu4cpp::createInstance(const std::string &instanceName, const std::string &fmuResourceLocation) {
    return std::make_unique<Model>(instanceName, fmuResourceLocation);
}
