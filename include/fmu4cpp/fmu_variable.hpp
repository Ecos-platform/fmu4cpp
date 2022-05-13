
#ifndef FMU4CPP_FMU_VARIABLE_HPP
#define FMU4CPP_FMU_VARIABLE_HPP

#include <functional>
#include <optional>
#include <string>

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

        [[nodiscard]] virtual causality_t causality() const {
            return causality_;
        }

        [[nodiscard]] virtual std::optional<variability_t> variability() const {
            return variability_;
        }

        [[nodiscard]] virtual std::optional<initial_t> initial() const {
            return std::nullopt;
        }

    protected:
        causality_t causality_ = causality_t::LOCAL;
        std::optional<variability_t> variability_;
        std::optional<initial_t> initial_;
    };

    using IntVariable = Variable<int>;
    using RealVariable = Variable<double>;
    using BoolVariable = Variable<bool>;
    using StringVariable = Variable<std::string>;

}// namespace fmu4cpp

#endif//FMU4CPP_FMU_VARIABLE_HPP
