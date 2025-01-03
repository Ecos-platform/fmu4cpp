
#ifndef FMU4CPP_FMU_VARIABLE_HPP
#define FMU4CPP_FMU_VARIABLE_HPP

#include "variable_access.hpp"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace fmu4cpp {

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

    std::string to_string(const causality_t &c);
    std::string to_string(const variability_t &v);
    std::string to_string(const initial_t &i);


    class VariableBase {

    protected:
        causality_t causality_ = causality_t::LOCAL;
        std::optional<variability_t> variability_;
        std::optional<initial_t> initial_;
        std::vector<std::string> annotations_;
        std::vector<size_t> dependencies_;

    public:
        VariableBase(std::string name, unsigned int vr, size_t index)
            : name_(std::move(name)), vr_(vr), index_(index) {}

        [[nodiscard]] const std::string &name() const {
            return name_;
        }

        [[nodiscard]] unsigned int value_reference() const {
            return vr_;
        }

        [[nodiscard]] size_t index() const {
            return index_;
        }

        [[nodiscard]] causality_t causality() const {
            return causality_;
        }

        [[nodiscard]] std::optional<variability_t> variability() const {
            return variability_;
        }

        [[nodiscard]] std::optional<initial_t> initial() const {
            return initial_;
        }

        [[nodiscard]] std::vector<size_t> getDependencies() const {
            if (causality_ != causality_t::OUTPUT) {
                throw std::logic_error("Can only declare dependencies for outputs!");
            }
            return dependencies_;
        }

        [[nodiscard]] std::vector<std::string> getAnnotations() const {
            return annotations_;
        }

        virtual ~VariableBase() = default;

    private:
        std::string name_;
        unsigned int vr_;
        size_t index_;
    };

    template<class T, class V>
    class Variable : public VariableBase {

    public:
        Variable(
                const std::string &name,
                unsigned int vr, size_t index, T *ptr)
            : VariableBase(name, vr, index), access_(std::make_unique<PtrAccess<T>>(ptr)) {}

        Variable(
                const std::string &name,
                unsigned int vr, size_t index,
                std::function<T()> getter,
                std::optional<std::function<void(T)>> setter)
            : VariableBase(name, vr, index),
              access_(std::make_unique<LambdaAccess<T>>(std::move(getter), std::move(setter))) {}

        [[nodiscard]] T get() const {

            return access_->get();
        }

        void set(T value) {
            if (causality_ == causality_t::LOCAL || causality_ == causality_t::OUTPUT) {
                throw std::logic_error("Cannot set value for variable with causality: " + to_string(causality_));
            }

            access_->set(value);
        }

        V &setCausality(causality_t causality) {
            causality_ = causality;
            return *static_cast<V *>(this);
        }

        V &setVariability(variability_t variability) {
            variability_ = variability;
            return *static_cast<V *>(this);
        }

        V &setInitial(initial_t initial) {
            initial_ = initial;
            return *static_cast<V *>(this);
        }

        V &setDependencies(const std::vector<size_t> &dependencies) {
            for (auto i: dependencies) {
                dependencies_.emplace_back(i);
            }
            return *static_cast<V *>(this);
        }

        V &setAnnotations(const std::vector<std::string> &annotations) {
            for (auto i: annotations) {
                annotations_.emplace_back(i);
            }
            return *static_cast<V *>(this);
        }

        V &addAnnotation(const std::string &annotation) {
            annotations_.emplace_back(annotation);
            return *static_cast<V *>(this);
        }

    private:
        std::shared_ptr<VariableAccess<T>> access_;
    };

    class IntVariable final : public Variable<int, IntVariable> {

    public:
        IntVariable(
                const std::string &name,
                unsigned int vr, size_t index, int *ptr)
            : Variable(name, vr, index, ptr) {}

        IntVariable(
                const std::string &name,
                unsigned int vr, size_t index,
                const std::function<int()> &getter,
                const std::optional<std::function<void(int)>> &setter)
            : Variable(name, vr, index, getter, setter) {}

        [[nodiscard]] std::optional<int> getMin() const {
            return min_;
        }

        [[nodiscard]] std::optional<int> getMax() const {
            return max_;
        }

        IntVariable &setMin(const std::optional<int> &min) {
            min_ = min;
            return *this;
        }

        IntVariable &setMax(const std::optional<int> &max) {
            max_ = max;
            return *this;
        }

    private:
        std::optional<int> min_;
        std::optional<int> max_;
    };

    class RealVariable final : public Variable<double, RealVariable> {

    public:
        RealVariable(
                const std::string &name,
                unsigned int vr, size_t index, double *ptr)
            : Variable(name, vr, index, ptr) {

            variability_ = variability_t::CONTINUOUS;
        }

        RealVariable(
                const std::string &name,
                unsigned int vr, size_t index,
                const std::function<double()> &getter,
                const std::optional<std::function<void(double)>> &setter)
            : Variable(name, vr, index, getter, setter) {

            variability_ = variability_t::CONTINUOUS;
        }

        [[nodiscard]] std::optional<double> getMin() const {
            return min_;
        }

        [[nodiscard]] std::optional<double> getMax() const {
            return max_;
        }

        RealVariable &setMin(const std::optional<double> &min) {
            min_ = min;
            return *this;
        }

        RealVariable &setMax(const std::optional<double> &max) {
            max_ = max;
            return *this;
        }

    private:
        std::optional<double> min_;
        std::optional<double> max_;
    };

    class BoolVariable final : public Variable<bool, BoolVariable> {

    public:
        BoolVariable(
                const std::string &name,
                unsigned int vr, size_t index, bool *ptr)
            : Variable(name, vr, index, ptr) {}

        BoolVariable(
                const std::string &name,
                unsigned int vr, size_t index,
                const std::function<bool()> &getter,
                const std::optional<std::function<void(bool)>> &setter)
            : Variable(name, vr, index, getter, setter) {}
    };

    class StringVariable final : public Variable<std::string, StringVariable> {

    public:
        StringVariable(
                const std::string &name,
                unsigned int vr, size_t index, std::string *ptr)
            : Variable(name, vr, index, ptr) {}

        StringVariable(
                const std::string &name,
                unsigned int vr, size_t index,
                const std::function<std::string()> &getter,
                const std::optional<std::function<void(std::string)>> &setter)
            : Variable(name, vr, index, getter, setter) {}
    };

    bool requires_start(const VariableBase &v);

}// namespace fmu4cpp

#endif//FMU4CPP_FMU_VARIABLE_HPP
