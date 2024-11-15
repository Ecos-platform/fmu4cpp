

#include <fmu4cpp/fmu_base.hpp>

#include <fstream>

using namespace fmu4cpp;


class Resource : public fmu_base {

public:
    Resource(const std::string &instanceName, const std::filesystem::path &resources)
        : fmu_base(instanceName, resources) {

        std::ifstream ifs(resources / "file.txt");

        std::string line;
        std::getline(ifs, line);

        register_variable(
                string(
                        "content", [line] { return line; })
                        .setVariability(variability_t::CONSTANT)
                        .setCausality(causality_t::OUTPUT));

        Resource::reset();
    }

    bool do_step(double currentTime, double dt) override {

        debugLog(fmi2OK, get_string_variable("content")->get());
        return true;
    }

    void reset() override {
        // do nothing
    }

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