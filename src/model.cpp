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

        register_int(
                "integerIn", [this] { return integer; }, [this](int value) { integer = value; })
                .setCausality(causality_t::INPUT);
        register_real(
                "realIn", [this] { return real; }, [this](double value) { real = value; })
                .setCausality(causality_t::INPUT);
        register_bool(
                "booleanIn", [this] { return boolean; }, [this](bool value) { boolean = value; })
                .setCausality(causality_t::INPUT);
        register_string(
                "stringIn", [this] { return string; }, [this](std::string value) { string = std::move(value); })
                .setCausality(causality_t::INPUT);

        register_int(
                "integerOut", [this] { return integer; })
                .setCausality(causality_t::OUTPUT);
        register_real(
                "realOut", [this] { return real; })
                .setCausality(causality_t::OUTPUT);
        register_bool(
                "booleanOut", [this] { return boolean; })
                .setCausality(causality_t::OUTPUT);
        register_string(
                "stringOut", [this] { return string; })
                .setCausality(causality_t::OUTPUT);

        Model::reset();
    }

    bool do_step(double currentTime, double dt) override {
        return true;
    }

    void reset() override {
        integer = 0;
        real = 0;
        boolean = false;
        string = "";
    }

private:
    int integer;
    double real;
    bool boolean;
    std::string string;
};

std::unique_ptr<fmu_base> fmu4cpp::createInstance(const std::string &instanceName, const std::string &fmuResourceLocation) {
    return std::make_unique<Model>(instanceName, fmuResourceLocation);
}
