
#include <fmu4cpp/fmu_base.hpp>

#include <cmath>

using namespace fmu4cpp;

class VanDerPol : public fmu_base {
public:
    FMU4CPP_CTOR(VanDerPol) {

        register_variable(real("x0", &x0_).setCausality(causality_t::OUTPUT).setInitial(initial_t::EXACT));
        register_variable(real("x1", &x1_).setCausality(causality_t::OUTPUT).setInitial(initial_t::EXACT));
        register_variable(real("mu", &mu_).setCausality(causality_t::PARAMETER).setVariability(variability_t::FIXED).setInitial(initial_t::EXACT));

        register_variable(real("x0Der", &x0Der_).setCausality(causality_t::LOCAL).setInitial(initial_t::CALCULATED).setDerivative("x0", {"x0"}));
        register_variable(real("x1Der", &x1Der_).setCausality(causality_t::LOCAL).setInitial(initial_t::CALCULATED).setDerivative("x1", {"x0", "x1"}));

        VanDerPol::reset();
    }

    bool do_step(double dt) override {
        if (dt <= 0.0) return false;

        // derivatives:
        // x' = y
        // y' = mu * (1 - x^2) * y - x
        auto dx = [&](double x, double y) { return y; };
        auto dy = [&](double x, double y) { return mu_ * (1.0 - x * x) * y - x; };

        // RK4
        double k1x = dx(x0_, x1_);
        double k1y = dy(x0_, x1_);

        double x2 = x0_ + 0.5 * dt * k1x;
        double y2 = x1_ + 0.5 * dt * k1y;
        double k2x = dx(x2, y2);
        double k2y = dy(x2, y2);

        double x3 = x0_ + 0.5 * dt * k2x;
        double y3 = x1_ + 0.5 * dt * k2y;
        double k3x = dx(x3, y3);
        double k3y = dy(x3, y3);

        double x4 = x0_ + dt * k3x;
        double y4 = x1_ + dt * k3y;
        double k4x = dx(x4, y4);
        double k4y = dy(x4, y4);

        x0Der_ = k1x;
        x1Der_ = k1y;

        x0_ += (dt / 6.0) * (k1x + 2.0 * k2x + 2.0 * k3x + k4x);
        x1_ += (dt / 6.0) * (k1y + 2.0 * k2y + 2.0 * k3y + k4y);

        return true;
    }


    void reset() override {
        // default initial conditions
        x0_ = 2.0;
        x1_ = 0.0;
        mu_ = 1.0;// stiffness parameter; adjust if needed

        x0Der_ = 0.0;
        x1Der_ = 0.0;
    }

private:
    double x0_{0.0};
    double x1_{0.0};
    double mu_{1.0};

    double x0Der_{};
    double x1Der_{};
};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "VanDerPol";
    info.description = "VanDerPol oscillator";
    info.modelIdentifier = FMU4CPP_MODEL_IDENTIFIER;
    return info;
}

FMU4CPP_INSTANTIATE(VanDerPol);
