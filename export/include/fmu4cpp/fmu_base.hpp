
#ifndef FMU4CPP_FMU_BASE_HPP
#define FMU4CPP_FMU_BASE_HPP

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "fmu_base.hpp"
#include "fmu_variable.hpp"
#include "logger.hpp"
#include "model_info.hpp"

#include <filesystem>

#define FMU4CPP_INSTANTIATE(MODELCLASS)                                                                            \
    std::unique_ptr<fmu4cpp::fmu_base> fmu4cpp::createInstance(const std::string &instanceName,                    \
                                                               const std::filesystem::path &fmuResourceLocation) { \
        return std::make_unique<MODELCLASS>(instanceName, fmuResourceLocation);                                    \
    }

namespace fmu4cpp {

    class fmu_base {

    public:
        fmu_base(std::string instance_name, std::filesystem::path resourceLocation)
            : instanceName_(std::move(instance_name)), resourceLocation_(std::move(resourceLocation)) {

            register_variable(real("time", &time_)
                                      .setCausality(causality_t::INDEPENDENT)
                                      .setVariability(variability_t::CONTINUOUS));
        }

        fmu_base(const fmu_base &) = delete;
        fmu_base(const fmu_base &&) = delete;

        [[nodiscard]] std::string instanceName() const {
            return instanceName_;
        }

        [[nodiscard]] const std::filesystem::path &resourceLocation() const {
            return resourceLocation_;
        }

        [[nodiscard]] std::optional<IntVariable> get_int_variable(const std::string &name) const {
            for (const auto &v: integers_) {
                if (v.name() == name) return v;
            }
            return std::nullopt;
        }

        [[nodiscard]] std::optional<RealVariable> get_real_variable(const std::string &name) const {
            for (const auto &v: reals_) {
                if (v.name() == name) return v;
            }
            return std::nullopt;
        }

        [[nodiscard]] std::optional<BoolVariable> get_bool_variable(const std::string &name) const {
            for (const auto &v: booleans_) {
                if (v.name() == name) return v;
            }
            return std::nullopt;
        }

        [[nodiscard]] std::optional<StringVariable> get_string_variable(const std::string &name) const {
            for (const auto &v: strings_) {
                if (v.name() == name) return v;
            }
            return std::nullopt;
        }

        void enter_initialisation_mode(double start, std::optional<double> stop, std::optional<double> tolerance) {
            time_ = start;
            stop_ = stop;
            tolerance_ = tolerance;
            enter_initialisation_mode();
        }

        virtual void exit_initialisation_mode();

        bool step(double currentTime, double dt) {

            if (stop_ && currentTime >= *stop_) {
                log(fmiWarning, "Stop time reached");
                return false;
            }

            constexpr double TIME_TOLERANCE = 1e-9;
            if (std::abs(currentTime - time_) > TIME_TOLERANCE) {
                throw std::runtime_error("Current time does not match the internal time (within tolerance)");
            }

            if (do_step(dt)) {
                time_ += dt;

                return true;
            }

            return false;
        }

        virtual void terminate();

        virtual void reset();

        void get_integer(const unsigned int vr[], size_t nvr, int value[]) const {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                const auto idx = vrToIntegerIndices_.at(ref);
                value[i] = integers_[idx].get();
            }
        }

        void get_real(const unsigned int vr[], size_t nvr, double value[]) const {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                const auto idx = vrToRealIndices_.at(ref);
                value[i] = reals_[idx].get();
            }
        }

        //fmi2
        void get_boolean(const unsigned int vr[], size_t nvr, int value[]) const {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                const auto idx = vrToBooleanIndices_.at(ref);
                value[i] = static_cast<int>(booleans_[idx].get());
            }
        }

        //fmi3
        void get_boolean(const unsigned int vr[], size_t nvr, bool value[]) const {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                const auto idx = vrToBooleanIndices_.at(ref);
                value[i] = booleans_[idx].get();
            }
        }

        void get_string(const unsigned int vr[], size_t nvr, const char *value[]) {
            stringBuffer_.clear();
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                const auto idx = vrToStringIndices_.at(ref);
                stringBuffer_.push_back(strings_[idx].get());
                value[i] = stringBuffer_.back().c_str();
            }
        }

        void set_integer(const unsigned int vr[], size_t nvr, const int value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                const auto idx = vrToIntegerIndices_.at(ref);
                integers_[idx].set(value[i]);
            }
        }

        void set_real(const unsigned int vr[], size_t nvr, const double value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                const auto idx = vrToRealIndices_.at(ref);
                reals_[idx].set(value[i]);
            }
        }

        //fmi2
        void set_boolean(const unsigned int vr[], size_t nvr, const int value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                const auto idx = vrToBooleanIndices_.at(ref);
                booleans_[idx].set(static_cast<bool>(value[i]));
            }
        }

        //fmi3
        void set_boolean(const unsigned int vr[], size_t nvr, const bool value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                const auto idx = vrToBooleanIndices_.at(ref);
                booleans_[idx].set(value[i]);
            }
        }

        void set_string(const unsigned int vr[], size_t nvr, const char *const value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                const auto idx = vrToStringIndices_.at(ref);
                strings_[idx].set(value[i]);
            }
        }

        [[nodiscard]] std::string guid() const;

        [[nodiscard]] std::string make_description_v2() const;

        [[nodiscard]] std::string make_description_v3() const;

        void __set_logger(logger *logger) {
            logger_ = logger;
        }

        void log(const fmiStatus s, const std::string &message) const {
            if (logger_) {
                logger_->log(s, message);
            }
        }

        virtual void *getFMUState() {

            return nullptr;
        }

        virtual bool setFmuState(void *state) {

            return false;
        }

        virtual bool freeFmuState(void **state) {

            return false;
        }

        virtual ~fmu_base() = default;

    protected:
        IntVariable integer(const std::string &name, int *ptr, const std::function<void(int)> &onChange = {});
        IntVariable integer(const std::string &name,
                            const std::function<int()> &getter,
                            const std::optional<std::function<void(int)>> &setter = std::nullopt);

        RealVariable real(const std::string &name, double *ptr, const std::function<void(double)> &onChange = {});
        RealVariable real(const std::string &name,
                          const std::function<double()> &getter,
                          const std::optional<std::function<void(double)>> &setter = std::nullopt);

        BoolVariable boolean(const std::string &name, bool *ptr, const std::function<void(bool)> &onChange = {});
        BoolVariable boolean(const std::string &name,
                             const std::function<bool()> &getter,
                             const std::optional<std::function<void(bool)>> &setter);

        StringVariable string(const std::string &name, std::string *ptr, const std::function<void(std::string)> &onChange = {});
        StringVariable string(const std::string &name,
                              const std::function<std::string()> &getter,
                              const std::optional<std::function<void(std::string)>> &setter = std::nullopt);

        void register_variable(IntVariable v);
        void register_variable(RealVariable v);
        void register_variable(BoolVariable v);
        void register_variable(StringVariable v);

        virtual void enter_initialisation_mode();
        virtual bool do_step(double dt) = 0;

        [[nodiscard]] double currentTime() const {
            return time_;
        }

        [[nodiscard]] std::optional<double> tolerance() const {
            return tolerance_;
        }

    private:
        double time_{0};
        std::optional<double> stop_;
        std::optional<double> tolerance_;

        logger *logger_ = nullptr;
        size_t numVariables_{0};

        std::string instanceName_;
        std::filesystem::path resourceLocation_;

        std::vector<IntVariable> integers_;
        std::unordered_map<unsigned int, size_t> vrToIntegerIndices_;

        std::vector<RealVariable> reals_;
        std::unordered_map<unsigned int, size_t> vrToRealIndices_;

        std::vector<BoolVariable> booleans_;
        std::unordered_map<unsigned int, size_t> vrToBooleanIndices_;

        std::vector<StringVariable> strings_;
        std::vector<std::string> stringBuffer_;
        std::unordered_map<unsigned int, size_t> vrToStringIndices_;
    };

    model_info get_model_info();

    std::unique_ptr<fmu_base> createInstance(const std::string &instanceName, const std::filesystem::path &fmuResourceLocation);

}// namespace fmu4cpp

#endif//FMU4CPP_FMU_BASE_HPP
