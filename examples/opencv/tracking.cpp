
#include "fmu4cpp/fmu_base.hpp"

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

class Tracking : public fmu4cpp::fmu_base {
public:
    FMU4CPP_CTOR(Tracking) {

        const auto onnxPath = resourceLocation() / "yolo11n.onnx";

        net_ = cv::dnn::readNetFromONNX(onnxPath.string());
        net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

        register_binary("blob", &binary_)
                .setCausality(fmu4cpp::causality_t::INPUT);

        Tracking::reset();
    }

    bool do_step(double) override {

        cv::Size size(640, 640);

        cv::Mat frame;
        if (!binary_.empty()) {
            frame = cv::imdecode(binary_, cv::IMREAD_COLOR);

            int height = frame.rows;
            int width = frame.cols;
            int length = std::max(height, width);
            cv::Mat image = cv::Mat::zeros(length, length, CV_8UC3);
            frame.copyTo(image(cv::Rect(0, 0, width, height)));

            // Create a 4D blob from the frame
            blobFromImage(image, blob, 1 / 255.0, size, {}, true, false);
            net_.setInput(blob);

            // Run forward pass
            std::vector<cv::Mat> outs;
            net_.forward(outs);

            // Extract the first output and reshape if necessary
            cv::Mat output = outs.front();

            // If output is not 2D, reshape it (e.g., for YOLO, it may be 3D: [1, N, M])
            if (output.dims > 2) {
                output = output.reshape(1, output.size[1]); // Flatten to 2D if required
            }

            // Transpose if needed (verify first)
            const cv::Mat &transposedOutput = output.t(); // Transpose output for processing
            int rows = transposedOutput.rows;


            // Post-process detections
            std::vector<int> classIds;
            std::vector<float> confidences;
            std::vector<cv::Rect> boxes;
        }
        return true;
    }

    void reset() override {
        // do nothing
    }

private:
    cv::dnn::Net net_;

    std::string window_ = "Tracking";
    std::vector<uint8_t> binary_;
};

fmu4cpp::model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Tracking";
    info.description = "A tracking model using ONNX and OpenCV";
    return info;
}


FMU4CPP_INSTANTIATE(Tracking);