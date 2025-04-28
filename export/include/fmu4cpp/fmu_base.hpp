
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

#define FMU4CPP_INSTANTIATE(MODELCLASS)                                                         \
    std::unique_ptr<fmu4cpp::fmu_base> fmu4cpp::createInstance(const fmu4cpp::fmu_data &data) { \
        return std::make_unique<MODELCLASS>(data);                                              \
    }

namespace fmu4cpp {

    struct fmu_data {
        logger *logger;
        std::string instance_name;
        std::filesystem::path resourceLocation;
    };

    class fmu_base {

    public:
        explicit fmu_base(fmu_data data)
            : data_(std::move(data)) {}

        fmu_base(const fmu_base &) = delete;
        fmu_base(const fmu_base &&) = delete;

        [[nodiscard]] std::string instanceName() const {
            return data_.instance_name;
        }

        [[nodiscard]] const std::filesystem::path &resourceLocation() const {
            return data_.resourceLocation;
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

        virtual void setup_experiment(double start, std::optional<double> stop, std::optional<double> tolerance);

        virtual void enter_initialisation_mode();

        virtual void exit_initialisation_mode();

        virtual bool do_step(double currentTime, double dt) = 0;

        virtual void terminate();

        virtual void reset();

        void get_integer(const unsigned int vr[], size_t nvr, int value[]) const {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                value[i] = integers_[ref].get();
            }
        }

        void get_real(const unsigned int vr[], size_t nvr, double value[]) const {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                value[i] = reals_[ref].get();
            }
        }

        void get_boolean(const unsigned int vr[], size_t nvr, int value[]) const {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                value[i] = static_cast<int>(booleans_[ref].get());
            }
        }

        void get_string(const unsigned int vr[], size_t nvr, const char *value[]) {
            stringBuffer_.clear();
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                stringBuffer_.push_back(strings_[ref].get());
                value[i] = stringBuffer_.back().c_str();
            }
        }

        void set_integer(const unsigned int vr[], size_t nvr, const int value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                integers_[ref].set(value[i]);
            }
        }

        void set_real(const unsigned int vr[], size_t nvr, const double value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                reals_[ref].set(value[i]);
            }
        }

        void set_boolean(const unsigned int vr[], size_t nvr, const int value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                booleans_[ref].set(static_cast<bool>(value[i]));
            }
        }

        void set_string(const unsigned int vr[], size_t nvr, const char *const value[]) {
            for (unsigned i = 0; i < nvr; i++) {
                const auto ref = vr[i];
                strings_[ref].set(value[i]);
            }
        }

        [[nodiscard]] std::string guid() const;

        [[nodiscard]] std::string make_description() const;

        void log(const fmi2Status s, const std::string &message) {
            if (data_.logger) {
                data_.logger->log(s, message);
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
        IntVariable integer(const std::string &name, int *ptr);
        IntVariable integer(const std::string &name,
                            const std::function<int()> &getter,
                            const std::optional<std::function<void(int)>> &setter = std::nullopt);

        RealVariable real(const std::string &name, double *ptr);
        RealVariable real(const std::string &name,
                          const std::function<double()> &getter,
                          const std::optional<std::function<void(double)>> &setter = std::nullopt);

        BoolVariable boolean(const std::string &name, bool *ptr);
        BoolVariable boolean(const std::string &name,
                             const std::function<bool()> &getter,
                             const std::optional<std::function<void(bool)>> &setter);

        StringVariable string(const std::string &name, std::string *ptr);
        StringVariable string(const std::string &name,
                              const std::function<std::string()> &getter,
                              const std::optional<std::function<void(std::string)>> &setter = std::nullopt);

        void register_variable(IntVariable v);
        void register_variable(RealVariable v);
        void register_variable(BoolVariable v);
        void register_variable(StringVariable v);


    private:
        fmu_data data_;
        size_t numVariables_{1};

        std::vector<IntVariable> integers_;
        std::vector<RealVariable> reals_;
        std::vector<BoolVariable> booleans_;
        std::vector<StringVariable> strings_;

        std::vector<std::string> stringBuffer_;
    };

    model_info get_model_info();

    std::unique_ptr<fmu_base> createInstance(const fmu_data &data);

}// namespace fmu4cpp

#endif//FMU4CPP_FMU_BASE_HPP
