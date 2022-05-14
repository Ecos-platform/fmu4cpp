//
// FMU4cpp Hello World exaple
//

#include <fmu4cpp/fmu_base.hpp>

using namespace fmu4cpp;

class Identity : public fmu_base {

public:
    Identity(const std::string &instanceName, const std::string &resources)
        : fmu_base(instanceName, resources) {

        register_int("integer", [this] { return integer; }, [this](int value) {integer = value;});
        register_real("real", [this] { return real; }, [this](double value) {integer = value;}).setCausality(causality_t::INPUT);
        register_bool("boolean", [this] { return boolean; }, [this](bool value) {boolean = value;});
        register_string("string", [this] { return string; }, [this](std::string value) {string = value;});
    }

    bool do_step(double currentTime, double dt) override {
        return true;
    }

private:
    int integer = 0;
    double real = 1;
    bool boolean = false;
    std::string string;
};

std::unique_ptr<fmu_base> fmu4cpp::createInstance(const std::string &instanceName, const std::string &fmuResourceLocation) {
    return std::make_unique<Identity>(instanceName, fmuResourceLocation);
}
