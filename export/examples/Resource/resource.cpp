

#include <fmu4cpp/fmu_base.hpp>

#include <fstream>

using namespace fmu4cpp;


class Resource : public fmu_base {

public:
    FMU4CPP_CTOR(Resource) {

        std::ifstream ifs(resourceLocation() / "file.txt");

        std::getline(ifs, content_);

        register_variable(
                string(
                        "content", &content_)
                        .setVariability(variability_t::CONSTANT)
                        .setCausality(causality_t::OUTPUT));

        Resource::reset();
    }

    bool do_step(double dt) override {

        log(fmiOK, get_string_variable("content")->get());
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

FMU4CPP_INSTANTIATE(Resource);
