
#ifndef FMU4CPP_FMU_VARIABLE_HPP
#define FMU4CPP_FMU_VARIABLE_HPP

#include <functional>
#include <optional>
#include <string>
#include <utility>

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

    std::string to_string(const causality_t& c);
    std::string to_string(const variability_t& v);
    std::string to_string(const initial_t& i);

    class VariableBase {
    private:
        std::string name_;
        unsigned int vr_;

    protected:
        causality_t causality_ = causality_t::LOCAL;
        std::optional<variability_t> variability_;
        std::optional<initial_t> initial_;

    public:
        VariableBase(std::string name, unsigned int vr) : name_(std::move(name)), vr_(vr) {}

        [[nodiscard]] const std::string &name() const {
            return name_;
        }

        [[nodiscard]] unsigned int value_reference() const {
            return vr_;
        }

        [[nodiscard]] causality_t causality() const {
            return causality_;
        }

        [[nodiscard]] std::optional<variability_t> variability() const {
            return variability_;
        }

        [[nodiscard]] std::optional<initial_t> initial() const {
            return std::nullopt;
        }

        bool requires_start() {
            return initial_ == initial_t::EXACT || initial_ == initial_t::APPROX || causality_ == causality_t::INPUT || causality_ == causality_t::PARAMETER || variability_ == variability_t::CONSTANT;
        }
    };

    template<class T>
    class Variable : public VariableBase {
    private:
        std::function<T()> getter_;
        std::optional<std::function<void(T)>> setter_;

    public:
        Variable(const std::string &name, unsigned int vr, std::function<T()> getter, std::optional<std::function<void(T)>> setter)
            : VariableBase(name, vr), getter_(std::move(getter)), setter_(std::move(setter)) {}

        [[nodiscard]] T get() const {
            return getter_();
        }

        void set(T value) {
            if (setter_) setter_->operator()(value);
        }
    };

    class IntVariable : public Variable<int> {
    private:
        std::optional<int> min_;
        std::optional<int> max_;

    public:
        IntVariable(
                const std::string &name,
                unsigned int vr,
                const std::function<int()> &getter,
                const std::optional<std::function<void(int)>> &setter)
            : Variable<int>(name, vr, getter, setter) {}

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

        IntVariable &setCausality(causality_t causality) {
            causality_ = causality;
            return *this;
        }

        IntVariable &setVariability(variability_t variability) {
            variability_ = variability;
            return *this;
        }

        IntVariable &setInitial(initial_t initial) {
            initial_ = initial;
            return *this;
        }
    };

    class RealVariable : public Variable<double> {
    private:
        std::optional<double> min_;
        std::optional<double> max_;

    public:
        RealVariable(
                const std::string &name,
                unsigned int vr,
                const std::function<double()> &getter,
                const std::optional<std::function<void(double)>> &setter)
            : Variable<double>(name, vr, getter, setter) {
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

        RealVariable &setCausality(causality_t causality) {
            causality_ = causality;
            return *this;
        }

        RealVariable &setVariability(variability_t variability) {
            variability_ = variability;
            return *this;
        }

        RealVariable &setInitial(initial_t initial) {
            initial_ = initial;
            return *this;
        }
    };

    class BoolVariable : public Variable<bool> {

    public:
        BoolVariable(
                const std::string &name,
                unsigned int vr,
                const std::function<bool()> &getter,
                const std::optional<std::function<void(bool)>> &setter)
            : Variable<bool>(name, vr, getter, setter) {}

        BoolVariable &setCausality(causality_t causality) {
            causality_ = causality;
            return *this;
        }

        BoolVariable &setVariability(variability_t variability) {
            variability_ = variability;
            return *this;
        }

        BoolVariable &setInitial(initial_t initial) {
            initial_ = initial;
            return *this;
        }
    };

    class StringVariable : public Variable<std::string> {

    public:
        StringVariable(
                const std::string &name,
                unsigned int vr,
                const std::function<std::string()> &getter,
                const std::optional<std::function<void(std::string)>> &setter)
            : Variable<std::string>(name, vr, getter, setter) {}

        StringVariable &setCausality(causality_t causality) {
            causality_ = causality;
            return *this;
        }

        StringVariable &setVariability(variability_t variability) {
            variability_ = variability;
            return *this;
        }

        StringVariable &setInitial(initial_t initial) {
            initial_ = initial;
            return *this;
        }
    };

    bool requires_start(const VariableBase& v);

}// namespace fmu4cpp

#endif//FMU4CPP_FMU_VARIABLE_HPP
