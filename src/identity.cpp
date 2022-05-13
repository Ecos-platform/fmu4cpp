//
// FMU4cpp Hello World exaple
//

#include <fmu4cpp/fmu_base.hpp>

using namespace fmu4cpp;

class Identity : public fmu_base {

public:
    Identity(const std::string &instanceName, const std::string &resources)
        : fmu_base(instanceName, resources) {

        register_int("integer", [this] { return integer; });
        register_real("real", [this] { return real; });
        register_bool("boolean", [this] { return boolean; });
        register_string("string", [this] { return string; });
    }

    bool do_step(double currentTime, double dt) override {
        return true;
    }

protected:
    [[nodiscard]] std::string modelName() const override {
        return "Identity";
    }

private:
    int integer = 0;
    double real = 0;
    bool boolean = false;
    std::string string;
};
