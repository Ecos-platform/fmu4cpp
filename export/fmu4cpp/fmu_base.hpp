
#ifndef FMU4CPP_FMU_BASE_HPP
#define FMU4CPP_FMU_BASE_HPP

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "fmu_base.hpp"
#include "fmu_except.hpp"
#include "fmu_variable.hpp"
#include "model_info.hpp"

namespace fmu4cpp {

    class fmu_base {

    public:
        fmu_base(std::string instance_name, std::string resourceLocation)
            : instanceName_(std::move(instance_name)), resourceLocation_(std::move(resourceLocation)) {}

        fmu_base(const fmu_base &) = delete;
        fmu_base(const fmu_base &&) = delete;

        [[nodiscard]] std::string instanceName() const {
            return instanceName_;
        }

        [[nodiscard]] std::string resourceLocation() const {
            return resourceLocation_;
        }

        std::optional<IntVariable> get_int_variable(const std::string &name) {
            for (auto &v: integers_) {
                if (v.name() == name) return v;
            }
            return std::nullopt;
        }

        std::optional<RealVariable> get_real_variable(const std::string &name) {
            for (auto &v: reals_) {
                if (v.name() == name) return v;
            }
            return std::nullopt;
        }

        std::optional<BoolVariable> get_bool_variable(const std::string &name) {
            for (auto &v: booleans_) {
                if (v.name() == name) return v;
            }
            return std::nullopt;
        }

        std::optional<StringVariable> get_string_variable(const std::string &name) {
            for (auto &v: strings_) {
                if (v.name() == name) return v;
            }
            return std::nullopt;
        }

        virtual void setup_experiment(double start, std::optional<double> stop, std::optional<double> tolerance);

        virtual void enter_initialisation_mode();

        virtual void exit_initialisation_mode();

        virtual bool do_step(double currentTime, double dt) = 0;

        virtual void terminate();

        virtual void reset();

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

        void get_string(const unsigned int vr[], size_t nvr, const char *value[]) const {
            for (unsigned i = 0; i < nvr; i++) {
                unsigned int ref = vr[i];
                value[i] = strings_[ref].get().c_str();
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

        void set_string(const unsigned int vr[], size_t nvr, const char *const value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                unsigned int ref = vr[i];
                booleans_[ref].set(value[i]);
            }
        }

        [[nodiscard]] std::string guid() const;

        [[nodiscard]] std::string make_description() const;

        virtual ~fmu_base() = default;

    protected:
        IntVariable integer(const std::string &name,
                            const std::function<int()> &getter,
                            const std::optional<std::function<void(int)>> &setter = std::nullopt);

        RealVariable real(const std::string &name,
                          const std::function<double()> &getter,
                          const std::optional<std::function<void(double)>> &setter = std::nullopt);

        BoolVariable boolean(const std::string &name,
                             const std::function<bool()> &getter,
                             const std::optional<std::function<void(bool)>> &setter = std::nullopt);

        StringVariable string(const std::string &name,
                              const std::function<std::string()> &getter,
                              const std::optional<std::function<void(std::string)>> &setter = std::nullopt);

        void register_variable(IntVariable v);
        void register_variable(RealVariable v);
        void register_variable(BoolVariable v);
        void register_variable(StringVariable v);

    private:
        size_t numVariables{};

        std::string instanceName_;
        std::string resourceLocation_;

        std::vector<IntVariable> integers_;
        std::vector<RealVariable> reals_;
        std::vector<BoolVariable> booleans_;
        std::vector<StringVariable> strings_;
    };

    model_info get_model_info();

    std::unique_ptr<fmu_base> createInstance(const std::string &instanceName, const std::string &fmuResourceLocation);

}// namespace fmu4cpp

#endif//FMU4CPP_FMU_BASE_HPP
