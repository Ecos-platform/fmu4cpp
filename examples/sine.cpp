
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

    void compute() {
        static double TWO_PI = 2 * atan(1) * 4;
        output = A * sin(TWO_PI * f * currentTime() + phi);
    }

public:
    sine(std::string instance_name, std::string resourceLocation)
        : fmu_base(std::move(instance_name), std::move(resourceLocation)) {
        register_real("A", [this] {
            return A;
        });
    }

protected:
    [[nodiscard]] std::string author() const override {
        return "John Doe";
    }

public:
    void exit_initialisation_mode() override {
        compute();
    }

    bool do_step(double currentTime, double dt) override {
        return true;
    }
};

int main() {

    sine s("", "");

    std::cout << s.make_description() << std::endl;
}
