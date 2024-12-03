

#include <fmu4cpp/fmu_base.hpp>

#include <fstream>

using namespace fmu4cpp;


class Resource : public fmu_base {

public:
    Resource(const std::string &instanceName, const std::filesystem::path &resources)
        : fmu_base(instanceName, resources) {

        std::ifstream ifs(resources / "file.txt");

        std::getline(ifs, content_);

        register_variable(
                string(
                        "content", &content_)
                        .setVariability(variability_t::CONSTANT)
                        .setCausality(causality_t::OUTPUT));

        Resource::reset();
    }

    bool do_step(double currentTime, double dt) override {

        log(fmi2OK, get_string_variable("content")->get());
        return true;
    }

    void reset() override {
        // do nothing
    }

private:
    std::string content_;

};

model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Resource";
    info.description = "A model with resources";
    info.modelIdentifier = FMU4CPP_MODEL_IDENTIFIER;
    return info;
}

std::unique_ptr<fmu_base> fmu4cpp::createInstance(const std::string &instanceName, const std::filesystem::path &fmuResourceLocation) {
    return std::make_unique<Resource>(instanceName, fmuResourceLocation);
}
