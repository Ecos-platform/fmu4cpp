
#include "fmu4cpp/fmu_base.hpp"

#include "nlohmann/json.hpp"

#include <iostream>

class ExternalInterfaceLib : public fmu4cpp::fmu_base {

public:
    FMU4CPP_CTOR(ExternalInterfaceLib) {

        // register_variable(integer("i", &i));
        register_variable(string("jsonString_", [this]{return jsonString_;}, [this](const std::string& val){jsonString_ = val;}));

        ExternalInterfaceLib::reset();
    }

    bool do_step(double dt) override {

        nlohmann::json json;
        json["time"] = currentTime();
        json["dt"] = dt;
        json["message"] = "This is an example of an external interface using JSON.";

        // ++i;

        jsonString_ = json.dump();

        std::cout << "Generated JSON string: " << jsonString_ << std::endl;

        log(fmiOK, "Generated JSON string: " + jsonString_);

        return true;
    }

    void reset() override {
        // i = 0;
        jsonString_ = "";
    }

private:
    // int i{};
    std::string jsonString_{};
};

fmu4cpp::model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "ExternalInterfaceLib";
    info.description = "An example FMU with an external interface using JSON.";
    return info;
}

FMU4CPP_INSTANTIATE(ExternalInterfaceLib)
