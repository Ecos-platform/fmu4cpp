
#ifndef FMU4CPP_FMU_BASE_HPP
#define FMU4CPP_FMU_BASE_HPP

#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace fmu4cpp {

    class fmu_base;

    std::unique_ptr<fmu_base> createInstance(const std::string &instanceName, const std::string &fmuResourceLocation);

    class fatal_error : public std::runtime_error {
    public:
        explicit fatal_error(const std::string &msg) : std::runtime_error(msg) {}
    };

    enum class causality_t {
        PARAMETER,
        CALCULATED_PARAMETER,
        INPUT,
        OUTPUT,
        LOCAL
    };


    enum class variability_t {
        CONSTANT,
        FIXED,
        TUNABLE,
        DISCRETE,
        CONTINUOUS
    };

    enum class initial_t {
        EXACT,
        APPROX,
        CALCULATED,
    };

    template<class T>
    class Variable {
    private:
        std::string name_;
        unsigned int vr_;
        std::function<T()> getter_;
        std::optional<std::function<void(T)>> setter_;

    public:
        Variable(std::string name, unsigned int vr, const std::function<T()> &getter, const std::optional<std::function<void(T)>> &setter)
            : name_(std::move(name)), vr_(vr), getter_(getter), setter_(setter) {}

        [[nodiscard]] const std::string &name() const {
            return name_;
        }

        [[nodiscard]] unsigned int value_reference() const {
            return vr_;
        }

        [[nodiscard]] T get() const {
            return getter_();
        }

        void set(T value) {
            if (setter_) setter_->operator()(value);
        }

        virtual causality_t causality() const {
            return causality_;
        }

        virtual variability_t variability() const {
            return variability_;
        }

        virtual std::optional<initial_t> initial() const {
            return std::nullopt;
        }

    protected:
        causality_t causality_ = causality_t::LOCAL;
        variability_t variability_ = variability_t::CONTINUOUS;
        std::optional<initial_t> initial_;
    };

    using IntVariable = Variable<int>;
    using RealVariable = Variable<double>;
    using BoolVariable = Variable<bool>;
    using StringVariable = Variable<std::string>;

    class fmu_base {

    public:
        fmu_base(std::string instance_name, std::string resourceLocation)
            : instanceName_(std::move(instance_name)), resourceLocation_(std::move(resourceLocation)) {}

        virtual void setup_experiment(double start, double stop, double tolerance);

        virtual void enter_initialisation_mode();

        virtual void exit_initialisation_mode();

        virtual bool do_step(double currentTime, double dt) = 0;

        virtual void terminate();

        void get_integer(const unsigned int vr[], size_t nvr, int value[]) const {
            for (unsigned i = 0; i < nvr; i++) {
                unsigned int ref = vr[i];
                value[i] = integers_[ref].get();
            }
        }

        void get_real(const unsigned int vr[], size_t nvr, double value[]) const {
            for (unsigned i = 0; i < nvr; i++) {
                unsigned int ref = vr[i];
                value[i] = reals_[ref].get();
            }
        }

        void get_boolean(const unsigned int vr[], size_t nvr, int value[]) const {
            for (unsigned i = 0; i < nvr; i++) {
                unsigned int ref = vr[i];
                value[i] = static_cast<int>(booleans_[ref].get());
            }
        }

        void set_integer(const unsigned int vr[], size_t nvr, const int value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                unsigned int ref = vr[i];
                integers_[ref].set(value[i]);
            }
        }

        void set_real(const unsigned int vr[], size_t nvr, const double value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                unsigned int ref = vr[i];
                reals_[ref].set(value[i]);
            }
        }

        void set_boolean(const unsigned int vr[], size_t nvr, const int value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                unsigned int ref = vr[i];
                booleans_[ref].set(static_cast<bool>(value[i]));
            }
        }

        [[nodiscard]] std::string make_description() const;

        ~fmu_base() = default;

    protected:
        [[nodiscard]] virtual std::string author() const { return ""; }
        [[nodiscard]] virtual std::string description() const { return ""; }
        [[nodiscard]] virtual std::string modelName() const { return ""; }

        [[nodiscard]] double currentTime() const {
            return currentTime_;
        }

        [[nodiscard]] std::string instanceName() const {
            return instanceName_;
        }

        [[nodiscard]] std::string resourceLocation() const {
            return resourceLocation_;
        }

        void register_int(
                const std::string &name,
                const std::function<int()> &getter,
                const std::optional<std::function<void(int)>> &setter = std::nullopt);

        void register_real(
                const std::string &name,
                const std::function<double()> &getter,
                const std::optional<std::function<void(double)>> &setter = std::nullopt);

        void register_bool(
                const std::string &name,
                const std::function<bool()> &getter,
                const std::optional<std::function<void(bool)>> &setter = std::nullopt);

        void register_string(
                const std::string &name,
                const std::function<std::string()> &getter,
                const std::optional<std::function<void(std::string)>> &setter = std::nullopt);

    private:
        double currentTime_ = 0;
        std::string resourceLocation_;
        std::string instanceName_;
        std::vector<IntVariable> integers_;
        std::vector<RealVariable> reals_;
        std::vector<BoolVariable> booleans_;
        std::vector<StringVariable> strings_;
    };

}// namespace fmu4cpp

#endif//FMU4CPP_FMU_BASE_HPP
