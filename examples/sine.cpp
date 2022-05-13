
#include <fmu4cpp/fmu_base.hpp>

#include <cmath>
#include <iostream>
#include <utility>

using namespace fmu4cpp;


class sine : public fmu_base {

private:
    double A = 1;
    double f = 0.1;
    double phi = 0;

    double output = 0;

    double t = 0;

    void compute() {
        static double TWO_PI = 2 * atan(1) * 4;
        output = A * sin(TWO_PI * f * t + phi);
    }

public:
    sine(std::string instance_name, const std::string& resourceLocation)
        : fmu_base(std::move(instance_name), resourceLocation) {
        register_real("A", [this] {
            return A;
        });
    }

protected:
    [[nodiscard]] std::string author() const override {
        return "John Doe";
    }

public:
    void setup_experiment(double start, std::optional<double> stop, std::optional<double> tolerance) override {
        t = start;
    }

    void exit_initialisation_mode() override {
        compute();
    }

    bool do_step(double currentTime, double dt) override {
        t = currentTime+dt;
        compute();
        return true;
    }
};

std::unique_ptr<fmu_base> fmu4cpp::createInstance(const std::string &instanceName, const std::string &fmuResourceLocation) {
    return std::make_unique<sine>(instanceName, fmuResourceLocation);
}

int main() {
    auto s = createInstance("", "");
    std::cout << s->make_description() << std::endl;
}
