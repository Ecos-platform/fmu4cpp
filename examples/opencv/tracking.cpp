
#include "fmu4cpp/fmu_base.hpp"


#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>

#include <fstream>
#include <iostream>

const std::vector<std::string> coco_names();

class BoxDrawer {
public:
    BoxDrawer(double confThreshold = 0.5, double nmsThreshold = 0.4)
        : confThreshold(confThreshold),
          nmsThreshold(nmsThreshold),
          classNames(coco_names()) {
    }

    void setConfThreshold(double confThreshold) {
        this->confThreshold = confThreshold;
    }

    void draw(cv::Mat &frame, const std::vector<int> &classIds,
              const std::vector<float> &confidences,
              const std::vector<cv::Rect> &boxes) const {
        // Non-maximum suppression to remove redundant overlapping boxes
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

        for (const int idx: indices) {
            const cv::Rect &box = boxes[idx];
            drawPred(classIds[idx], confidences[idx], box.x, box.y, box.x + box.width, box.y + box.height, frame,
                     classNames);
        }
    }

private:
    float confThreshold;
    float nmsThreshold;
    std::vector<std::string> classNames;

    static void drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat &frame,
                         const std::vector<std::string> &classNames) {
        rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(255, 178, 50), 3);

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << conf;
        std::string label = oss.str();
        if (!classNames.empty()) {
            CV_Assert(classId < classNames.size());
            label = classNames[classId] + ": " + label;
        }

        int baseLine;
        const auto labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        top = cv::max(top, labelSize.height);
        rectangle(frame, cv::Point(left, top - labelSize.height),
                  cv::Point(left + labelSize.width, top + baseLine), cv::Scalar::all(255), cv::FILLED);
        putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    }
};

class Tracking : public fmu4cpp::fmu_base {
public:
    FMU4CPP_CTOR(Tracking) {

        register_binary("blob", &binary_)
                .setCausality(fmu4cpp::causality_t::INPUT);

        register_real("confThreshold", &confThreshold)
                .setCausality(fmu4cpp::causality_t::PARAMETER)
                .setVariability(fmu4cpp::variability_t::TUNABLE)
                .setMin(0.0)
                .setMax(1.0);

        Tracking::reset();
    }

    void exit_initialisation_mode() override {
        const auto onnxPath = resourceLocation() / "yolo11n.onnx";

        if (!std::filesystem::exists(onnxPath)) {
            throw std::runtime_error("ONNX model file not found: " + onnxPath.string());
        }

        net_ = cv::dnn::readNetFromONNX(onnxPath.string());
        net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    bool do_step(double) override {

        static cv::Size size(640, 640);

        std::cout << "Running inference..." << std::endl;
        std::cout << binary_.size() << " bytes received." << std::endl;

        cv::Mat frame, blob;
        if (!binary_.empty()) {
            frame = cv::imdecode(binary_, cv::IMREAD_COLOR);

            int height = frame.rows;
            int width = frame.cols;
            std::cout << "Frame size: " << width << "x" << height << std::endl;
            int length = std::max(height, width);
            cv::Mat image = cv::Mat::zeros(length, length, CV_8UC3);
            frame.copyTo(image(cv::Rect(0, 0, width, height)));

            // Create a 4D blob from the frame
            cv::dnn::blobFromImage(image, blob, 1 / 255.0, size, {}, true, false);
            net_.setInput(blob);

            // Run forward pass
            std::vector<cv::Mat> outs;
            net_.forward(outs);

            // Extract the first output and reshape if necessary
            cv::Mat output = outs.front();

            // If output is not 2D, reshape it (e.g., for YOLO, it may be 3D: [1, N, M])
            if (output.dims > 2) {
                output = output.reshape(1, output.size[1]);// Flatten to 2D if required
            }

            // Transpose if needed (verify first)
            const cv::Mat &transposedOutput = output.t();// Transpose output for processing
            int rows = transposedOutput.rows;


            // Post-process detections
            std::vector<int> classIds;
            std::vector<float> confidences;
            std::vector<cv::Rect> boxes;

            for (int i = 0; i < rows; ++i) {
                cv::Mat scores = transposedOutput.row(i).colRange(4, transposedOutput.cols);
                cv::Point classIdPoint;
                double confidence;
                minMaxLoc(scores, nullptr, &confidence, nullptr, &classIdPoint);
                if (confidence > confThreshold) {
                    // Extract bounding box coordinates
                    float centerX = transposedOutput.at<float>(i, 0);
                    float centerY = transposedOutput.at<float>(i, 1);
                    float width = transposedOutput.at<float>(i, 2);
                    float height = transposedOutput.at<float>(i, 3);

                    // Calculate top-left corner of the bounding box
                    int x = static_cast<int>(centerX - 0.5 * width);
                    int y = static_cast<int>(centerY - 0.5 * height);
                    int w = static_cast<int>(width);
                    int h = static_cast<int>(height);

                    classIds.push_back(classIdPoint.x);
                    confidences.emplace_back(static_cast<float>(confidence));
                    boxes.emplace_back(x, y, w, h);
                }
            }

            std::cout << "Detections: " << boxes.size() << std::endl;

            // drawer.setConfThreshold(confThreshold);
            // drawer.draw(frame, classIds, confidences, boxes);
            // imshow(windowName_, frame);

        }
        return true;
    }

    void reset() override {
        // do nothing
    }

private:
    cv::dnn::Net net_;

    std::string windowName_ = "Tracking";
    std::vector<uint8_t> binary_;

    double confThreshold = 0.5;
    BoxDrawer drawer{};
};

const std::vector<std::string> coco_names() {
    static std::vector<std::string> names = {
            "person", "bicycle", "car", "motorbike", "aeroplane", "bus", "train", "truck", "boat", "traffic light",
            "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
            "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
            "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard",
            "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl",
            "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake",
            "chair", "sofa", "pottedplant", "bed", "diningtable", "toilet", "tvmonitor", "laptop", "mouse",
            "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator",
            "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"};
    return names;
}


fmu4cpp::model_info fmu4cpp::get_model_info() {
    model_info info;
    info.modelName = "Tracking";
    info.description = "A tracking model using ONNX and OpenCV";
    return info;
}


FMU4CPP_INSTANTIATE(Tracking);