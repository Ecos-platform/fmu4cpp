
#ifndef FMU4CPP_VARIABLEACCESS_HPP
#define FMU4CPP_VARIABLEACCESS_HPP

#include <functional>
#include <optional>
#include <utility>

namespace fmu4cpp {

    template<typename T>
    struct VariableAccess {
        virtual T get() = 0;
        virtual void set(T value) = 0;

        virtual ~VariableAccess() = default;
    };

    template<typename T>
    class PtrAccess final : public VariableAccess<T> {

    public:
        explicit PtrAccess(T *ptr) : ptr_(ptr) {}

        T get() override {
            return *ptr_;
        }

        void set(T value) override {
            *ptr_ = value;
        }

    private:
        T *ptr_;
    };

    template<typename T>
    class LambdaAccess final : public VariableAccess<T> {

    public:
        LambdaAccess(std::function<T()> getter, std::optional<std::function<void(T)>> setter)
            : getter_(std::move(getter)),
              setter_(std::move(setter)) {}

        T get() override {
            return getter_();
        }

        void set(T value) override {
            if (setter_) {
                setter_->operator()(value);
            }
        }

    private:
        std::function<T()> getter_;
        std::optional<std::function<void(T)>> setter_;
    };

}// namespace fmu4cpp

#endif//FMU4CPP_VARIABLEACCESS_HPP
