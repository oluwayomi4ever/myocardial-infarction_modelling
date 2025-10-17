#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <iostream>

// Include your existing MI modeling classes
#include "../include/FitzHughNagumo.h"
#include "../include/DTM.h"

using namespace emscripten;

// WebAssembly wrapper for FitzHugh-Nagumo model
class WASMFitzHughNagumo {
private:
    std::unique_ptr<FitzHughNagumo> model;
    std::vector<std::pair<int, double>> epochResults; // epoch, accuracy
    std::vector<double> trainingLoss;
    std::vector<double> validationLoss;
    int currentEpoch;
    bool trainingMode;
    
    // Classification metrics
    struct ClassificationMetrics {
        double accuracy;
        double precision;
        double recall;
        double f1Score;
        double specificity;
        double sensitivity;
        int truePositives;
        int falsePositives;
        int trueNegatives;
        int falseNegatives;
    };
    
    std::vector<ClassificationMetrics> classificationHistory;
    
public:
    WASMFitzHughNagumo(int width, int height, double dt = 0.01) {
        model = std::make_unique<FitzHughNagumo>(width, height, dt);
        currentEpoch = 0;
        trainingMode = false;
        epochResults.clear();
        trainingLoss.clear();
        validationLoss.clear();
    }
    
    void initialize() {
        model->initialize();
    }
    
    void setParameters(double a, double b, double c, double d) {
        model->setParameters(a, b, c, d);
    }
    
    void setDiffusionCoefficients(double du, double dv) {
        model->setDiffusionCoefficients(du, dv);
    }
    
    void addStimulus(int x, int y, double strength, double duration) {
        model->addStimulus(x, y, strength, duration);
    }
    
    void run(int steps) {
        model->run(steps);
    }
    
    void run(int steps, bool enableTraining = false) {
        if (enableTraining) {
            trainingMode = true;
            runTrainingEpoch(steps);
        } else {
            model->run(steps);
        }
    }
    
    void runTrainingEpoch(int steps) {
        if (!trainingMode) return;
        
        // Run simulation for this epoch
        model->run(steps);
        
        // Calculate accuracy for this epoch
        double accuracy = calculateEpochAccuracy();
        
        // Calculate classification metrics
        ClassificationMetrics metrics = calculateClassificationMetrics();
        
        // Store epoch results
        epochResults.push_back(std::make_pair(currentEpoch, accuracy));
        
        // Store classification metrics
        classificationHistory.push_back(metrics);
        
        // Calculate training and validation loss
        double trainLoss = calculateTrainingLoss();
        double valLoss = calculateValidationLoss();
        
        trainingLoss.push_back(trainLoss);
        validationLoss.push_back(valLoss);
        
        currentEpoch++;
    }
    
    void step() {
        model->step();
    }
    
    double getTime() const {
        return model->getTime();
    }
    
    // Calculate accuracy for current epoch based on physiological criteria
    double calculateEpochAccuracy() const {
        const auto& membraneData = model->getU();
        const auto& recoveryData = model->getV();
        
        double totalAccuracy = 0.0;
        int validCells = 0;
        
        for (size_t y = 0; y < membraneData.size(); y++) {
            for (size_t x = 0; x < membraneData[y].size(); x++) {
                double membrane = membraneData[y][x];
                double recovery = recoveryData[y][x];
                
                // Physiological accuracy criteria
                double accuracy = 1.0;
                
                // Check membrane potential range (-100mV to +50mV is physiologically reasonable)
                if (membrane < -100.0 || membrane > 50.0) {
                    accuracy *= 0.5; // Penalize unrealistic values
                }
                
                // Check recovery variable range (0.0 to 1.0 is typical)
                if (recovery < 0.0 || recovery > 1.0) {
                    accuracy *= 0.7;
                }
                
                // Check for stable oscillations (variance-based)
                if (y > 0 && y < membraneData.size() - 1 && 
                    x > 0 && x < membraneData[y].size() - 1) {
                    double localVariance = 0.0;
                    double mean = 0.0;
                    int count = 0;
                    
                    // Calculate local variance
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            mean += membraneData[y + dy][x + dx];
                            count++;
                        }
                    }
                    mean /= count;
                    
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            double diff = membraneData[y + dy][x + dx] - mean;
                            localVariance += diff * diff;
                        }
                    }
                    localVariance /= count;
                    
                    // Reward moderate variance (indicating healthy electrical activity)
                    if (localVariance > 0.01 && localVariance < 1.0) {
                        accuracy *= 1.1; // Bonus for good variance
                    } else if (localVariance > 2.0) {
                        accuracy *= 0.8; // Penalize excessive variance
                    }
                }
                
                totalAccuracy += accuracy;
                validCells++;
            }
        }
        
        return validCells > 0 ? (totalAccuracy / validCells) * 100.0 : 0.0;
    }
    
    double calculateTrainingLoss() const {
        // Calculate mean squared error between current state and target
        const auto& membraneData = model->getU();
        double loss = 0.0;
        int count = 0;
        
        // Target: stable resting potential around -80mV
        double targetPotential = -80.0;
        
        for (const auto& row : membraneData) {
            for (double value : row) {
                double error = value - targetPotential;
                loss += error * error;
                count++;
            }
        }
        
        return count > 0 ? loss / count : 0.0;
    }
    
    double calculateValidationLoss() const {
        // Calculate validation loss based on physiological constraints
        const auto& membraneData = model->getU();
        double loss = 0.0;
        int count = 0;
        
        for (const auto& row : membraneData) {
            for (double value : row) {
                // Penalize values outside physiological range
                if (value < -100.0 || value > 50.0) {
                    loss += 1.0; // High penalty
                } else if (value < -90.0 || value > 40.0) {
                    loss += 0.1; // Moderate penalty
                }
                count++;
            }
        }
        
        return count > 0 ? loss / count : 0.0;
    }
    
    // Generate ground truth data for MI classification
    std::vector<std::vector<bool>> generateGroundTruth() const {
        const auto& membraneData = model->getU();
        std::vector<std::vector<bool>> groundTruth;
        
        groundTruth.resize(membraneData.size());
        for (size_t y = 0; y < membraneData.size(); y++) {
            groundTruth[y].resize(membraneData[y].size());
            for (size_t x = 0; x < membraneData[y].size(); x++) {
                // Simulate MI regions (central area with reduced potential)
                double center_x = membraneData[y].size() / 2.0;
                double center_y = membraneData.size() / 2.0;
                double distance = std::sqrt((x - center_x) * (x - center_x) + (y - center_y) * (y - center_y));
                
                // MI region: distance < 20% of grid size
                bool isMI = distance < std::min(membraneData[y].size(), membraneData.size()) * 0.2;
                groundTruth[y][x] = isMI;
            }
        }
        
        return groundTruth;
    }
    
    // Classify tissue as MI or normal based on membrane potential
    std::vector<std::vector<bool>> classifyTissue() const {
        const auto& membraneData = model->getU();
        std::vector<std::vector<bool>> predictions;
        
        predictions.resize(membraneData.size());
        for (size_t y = 0; y < membraneData.size(); y++) {
            predictions[y].resize(membraneData[y].size());
            for (size_t x = 0; x < membraneData[y].size(); x++) {
                // Classification threshold: MI if potential < -60mV (depolarized)
                bool isMI = membraneData[y][x] < -60.0;
                predictions[y][x] = isMI;
            }
        }
        
        return predictions;
    }
    
    // Calculate comprehensive classification metrics
    ClassificationMetrics calculateClassificationMetrics() const {
        auto groundTruth = generateGroundTruth();
        auto predictions = classifyTissue();
        
        ClassificationMetrics metrics = {0};
        
        int tp = 0, fp = 0, tn = 0, fn = 0;
        
        for (size_t y = 0; y < groundTruth.size(); y++) {
            for (size_t x = 0; x < groundTruth[y].size(); x++) {
                bool actual = groundTruth[y][x];
                bool predicted = predictions[y][x];
                
                if (actual && predicted) tp++;
                else if (!actual && predicted) fp++;
                else if (!actual && !predicted) tn++;
                else if (actual && !predicted) fn++;
            }
        }
        
        metrics.truePositives = tp;
        metrics.falsePositives = fp;
        metrics.trueNegatives = tn;
        metrics.falseNegatives = fn;
        
        // Calculate metrics
        int total = tp + fp + tn + fn;
        metrics.accuracy = total > 0 ? (double)(tp + tn) / total : 0.0;
        
        metrics.precision = (tp + fp) > 0 ? (double)tp / (tp + fp) : 0.0;
        metrics.recall = (tp + fn) > 0 ? (double)tp / (tp + fn) : 0.0;
        metrics.sensitivity = metrics.recall;
        metrics.specificity = (tn + fp) > 0 ? (double)tn / (tn + fp) : 0.0;
        
        metrics.f1Score = (metrics.precision + metrics.recall) > 0 ? 
                         2.0 * (metrics.precision * metrics.recall) / (metrics.precision + metrics.recall) : 0.0;
        
        return metrics;
    }
    
    // Return membrane potential data as JavaScript array
    emscripten::val getMembranePotential() const {
        const auto& data = model->getU();
        emscripten::val result = emscripten::val::array();
        
        for (const auto& row : data) {
            emscripten::val jsRow = emscripten::val::array();
            for (double value : row) {
                jsRow.call<void>("push", value);
            }
            result.call<void>("push", jsRow);
        }
        
        return result;
    }
    
    // Return recovery variable data as JavaScript array
    emscripten::val getRecoveryVariable() const {
        const auto& data = model->getV();
        emscripten::val result = emscripten::val::array();
        
        for (const auto& row : data) {
            emscripten::val jsRow = emscripten::val::array();
            for (double value : row) {
                jsRow.call<void>("push", value);
            }
            result.call<void>("push", jsRow);
        }
        
        return result;
    }
    
    // Get dimensions
    emscripten::val getDimensions() const {
        emscripten::val result = emscripten::val::object();
        result.set("width", 100);  // Default for demo
        result.set("height", 100); // Default for demo
        return result;
    }
    
    // Save state to file
    bool saveState(const std::string& filename) const {
        return model->saveState(filename);
    }
    
    // Load state from file
    bool loadState(const std::string& filename) {
        return model->loadState(filename);
    }
    
    // Get epoch results as JavaScript array
    emscripten::val getEpochResults() const {
        emscripten::val result = emscripten::val::array();
        for (const auto& epoch : epochResults) {
            emscripten::val epochObj = emscripten::val::object();
            epochObj.set("epoch", epoch.first);
            epochObj.set("accuracy", epoch.second);
            result.call<void>("push", epochObj);
        }
        return result;
    }
    
    // Get training loss history
    emscripten::val getTrainingLoss() const {
        emscripten::val result = emscripten::val::array();
        for (double loss : trainingLoss) {
            result.call<void>("push", loss);
        }
        return result;
    }
    
    // Get validation loss history
    emscripten::val getValidationLoss() const {
        emscripten::val result = emscripten::val::array();
        for (double loss : validationLoss) {
            result.call<void>("push", loss);
        }
        return result;
    }
    
    // Get current epoch number
    int getCurrentEpoch() const {
        return currentEpoch;
    }
    
    // Get training statistics
    emscripten::val getTrainingStats() const {
        emscripten::val stats = emscripten::val::object();
        stats.set("currentEpoch", currentEpoch);
        stats.set("isTraining", trainingMode);
        
        if (!epochResults.empty()) {
            double totalAccuracy = 0.0;
            double maxAccuracy = 0.0;
            double minAccuracy = 100.0;
            
            for (const auto& epoch : epochResults) {
                totalAccuracy += epoch.second;
                maxAccuracy = std::max(maxAccuracy, epoch.second);
                minAccuracy = std::min(minAccuracy, epoch.second);
            }
            
            stats.set("averageAccuracy", totalAccuracy / epochResults.size());
            stats.set("maxAccuracy", maxAccuracy);
            stats.set("minAccuracy", minAccuracy);
            stats.set("totalEpochs", static_cast<int>(epochResults.size()));
        }
        
        if (!trainingLoss.empty() && !validationLoss.empty()) {
            stats.set("finalTrainingLoss", trainingLoss.back());
            stats.set("finalValidationLoss", validationLoss.back());
        }
        
        return stats;
    }
    
    // Start training mode
    void startTraining() {
        trainingMode = true;
        currentEpoch = 0;
        epochResults.clear();
        trainingLoss.clear();
        validationLoss.clear();
    }
    
    // Stop training mode
    void stopTraining() {
        trainingMode = false;
    }
    
    // Run multiple epochs
    void runEpochs(int numEpochs, int stepsPerEpoch) {
        startTraining();
        for (int i = 0; i < numEpochs; i++) {
            runTrainingEpoch(stepsPerEpoch);
        }
    }
    
    // Get current classification metrics
    emscripten::val getClassificationMetrics() const {
        if (classificationHistory.empty()) {
            return emscripten::val::object();
        }
        
        const auto& metrics = classificationHistory.back();
        emscripten::val result = emscripten::val::object();
        
        result.set("accuracy", metrics.accuracy);
        result.set("precision", metrics.precision);
        result.set("recall", metrics.recall);
        result.set("f1Score", metrics.f1Score);
        result.set("specificity", metrics.specificity);
        result.set("sensitivity", metrics.sensitivity);
        result.set("truePositives", metrics.truePositives);
        result.set("falsePositives", metrics.falsePositives);
        result.set("trueNegatives", metrics.trueNegatives);
        result.set("falseNegatives", metrics.falseNegatives);
        
        return result;
    }
    
    // Get confusion matrix data
    emscripten::val getConfusionMatrix() const {
        if (classificationHistory.empty()) {
            return emscripten::val::object();
        }
        
        const auto& metrics = classificationHistory.back();
        emscripten::val matrix = emscripten::val::array();
        
        // Create 2x2 confusion matrix
        emscripten::val row1 = emscripten::val::array();
        row1.call<void>("push", metrics.truePositives);
        row1.call<void>("push", metrics.falsePositives);
        matrix.call<void>("push", row1);
        
        emscripten::val row2 = emscripten::val::array();
        row2.call<void>("push", metrics.falseNegatives);
        row2.call<void>("push", metrics.trueNegatives);
        matrix.call<void>("push", row2);
        
        return matrix;
    }
    
    // Get classification metrics history for all epochs
    emscripten::val getClassificationHistory() const {
        emscripten::val result = emscripten::val::array();
        
        for (size_t i = 0; i < classificationHistory.size(); i++) {
            const auto& metrics = classificationHistory[i];
            emscripten::val epochMetrics = emscripten::val::object();
            
            epochMetrics.set("epoch", static_cast<int>(i));
            epochMetrics.set("accuracy", metrics.accuracy);
            epochMetrics.set("precision", metrics.precision);
            epochMetrics.set("recall", metrics.recall);
            epochMetrics.set("f1Score", metrics.f1Score);
            epochMetrics.set("specificity", metrics.specificity);
            epochMetrics.set("sensitivity", metrics.sensitivity);
            
            result.call<void>("push", epochMetrics);
        }
        
        return result;
    }
};

// WebAssembly wrapper for DTM solver
class WASMDTM {
private:
    std::unique_ptr<DTM> dtm;
    
public:
    WASMDTM(int width, int height, double spatial_step = 1.0) {
        dtm = std::make_unique<DTM>(width, height, spatial_step);
    }
    
    bool loadFromFile(const std::string& filename) {
        return dtm->loadFromFile(filename);
    }
    
    bool saveToFile(const std::string& filename) const {
        return dtm->saveToFile(filename);
    }
    
    double getElevation(int x, int y) const {
        return dtm->getElevation(x, y);
    }
    
    void setElevation(int x, int y, double elevation) {
        dtm->setElevation(x, y, elevation);
    }
    
    emscripten::val getDimensions() const {
        auto dims = dtm->getDimensions();
        emscripten::val result = emscripten::val::object();
        result.set("width", dims.first);
        result.set("height", dims.second);
        return result;
    }
    
    double getCellSize() const {
        return dtm->getCellSize();
    }
    
    double calculateSlope(int x, int y) const {
        return dtm->calculateSlope(x, y);
    }
    
    double calculateAspect(int x, int y) const {
        return dtm->calculateAspect(x, y);
    }
};

// File processing functions
class FileProcessor {
public:
    static emscripten::val processECGData(emscripten::val jsData) {
        // Convert JavaScript array to C++ vector
        std::vector<double> data;
        int length = jsData["length"].as<int>();
        
        for (int i = 0; i < length; i++) {
            data.push_back(jsData[i].as<double>());
        }
        
        // Process ECG data
        std::vector<double> processed;
        double mean = 0.0;
        double sum = 0.0;
        
        // Calculate mean
        for (double value : data) {
            sum += value;
        }
        mean = sum / data.size();
        
        // Apply simple high-pass filter (remove baseline)
        for (double value : data) {
            processed.push_back(value - mean);
        }
        
        // Return processed data as JavaScript array
        emscripten::val result = emscripten::val::array();
        for (double value : processed) {
            result.call<void>("push", value);
        }
        
        return result;
    }
    
    static emscripten::val detectRPeaks(emscripten::val jsData) {
        std::vector<double> data;
        int length = jsData["length"].as<int>();
        
        for (int i = 0; i < length; i++) {
            data.push_back(jsData[i].as<double>());
        }
        
        // Simple R-peak detection
        std::vector<int> peaks;
        double threshold = 0.0;
        double max_val = 0.0;
        
        // Find maximum value
        for (double value : data) {
            if (std::abs(value) > max_val) {
                max_val = std::abs(value);
            }
        }
        
        threshold = max_val * 0.7; // 70% of maximum
        
        // Detect peaks
        for (int i = 1; i < data.size() - 1; i++) {
            if (data[i] > threshold && 
                data[i] > data[i-1] && 
                data[i] > data[i+1]) {
                peaks.push_back(i);
            }
        }
        
        // Return peaks as JavaScript array
        emscripten::val result = emscripten::val::array();
        for (int peak : peaks) {
            result.call<void>("push", peak);
        }
        
        return result;
    }
    
    static emscripten::val calculateECGMetrics(emscripten::val jsData) {
        std::vector<double> data;
        int length = jsData["length"].as<int>();
        
        for (int i = 0; i < length; i++) {
            data.push_back(jsData[i].as<double>());
        }
        
        // Calculate metrics
        double min_val = *std::min_element(data.begin(), data.end());
        double max_val = *std::max_element(data.begin(), data.end());
        
        double sum = 0.0;
        for (double value : data) {
            sum += value;
        }
        double mean = sum / data.size();
        
        double variance = 0.0;
        for (double value : data) {
            variance += (value - mean) * (value - mean);
        }
        double std_dev = std::sqrt(variance / data.size());
        
        // Return metrics as JavaScript object
        emscripten::val result = emscripten::val::object();
        result.set("min", min_val);
        result.set("max", max_val);
        result.set("mean", mean);
        result.set("std", std_dev);
        result.set("range", max_val - min_val);
        
        return result;
    }
};

// Global simulation runner functions
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    double run_fitzhugh_nagumo_simulation(int width, int height, int steps, double dt) {
        FitzHughNagumo model(width, height, dt);
        model.initialize();
        model.setParameters(0.1, 0.5, 1.0, 0.0);
        model.setDiffusionCoefficients(0.1, 0.0);
        
        auto start = std::chrono::high_resolution_clock::now();
        model.run(steps);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return duration.count();
    }
    
    EMSCRIPTEN_KEEPALIVE
    double run_dtm_simulation(int width, int height, double spatial_step) {
        DTM dtm(width, height, spatial_step);
        
        // Fill with sample data
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                double center_x = width / 2.0;
                double center_y = height / 2.0;
                double distance = std::sqrt((x - center_x) * (x - center_x) + (y - center_y) * (y - center_y));
                double elevation = 100.0 * std::exp(-distance / 10.0);
                dtm.setElevation(x, y, elevation);
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Calculate terrain properties
        double total_slope = 0.0;
        int count = 0;
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                total_slope += dtm.calculateSlope(x, y);
                count++;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        return duration.count();
    }
}

// Emscripten bindings to expose classes to JavaScript
EMSCRIPTEN_BINDINGS(mi_modeling_wasm) {
    // FitzHugh-Nagumo bindings
    class_<WASMFitzHughNagumo>("FitzHughNagumo")
        .constructor<int, int, double>()
        .function("initialize", &WASMFitzHughNagumo::initialize)
        .function("setParameters", &WASMFitzHughNagumo::setParameters)
        .function("setDiffusionCoefficients", &WASMFitzHughNagumo::setDiffusionCoefficients)
        .function("addStimulus", &WASMFitzHughNagumo::addStimulus)
        .function("run", select_overload<void(int)>(&WASMFitzHughNagumo::run))
        .function("runWithTraining", select_overload<void(int, bool)>(&WASMFitzHughNagumo::run))
        .function("runEpochs", &WASMFitzHughNagumo::runEpochs)
        .function("step", &WASMFitzHughNagumo::step)
        .function("getTime", &WASMFitzHughNagumo::getTime)
        .function("getMembranePotential", &WASMFitzHughNagumo::getMembranePotential)
        .function("getRecoveryVariable", &WASMFitzHughNagumo::getRecoveryVariable)
        .function("getDimensions", &WASMFitzHughNagumo::getDimensions)
        .function("saveState", &WASMFitzHughNagumo::saveState)
        .function("loadState", &WASMFitzHughNagumo::loadState)
        .function("getEpochResults", &WASMFitzHughNagumo::getEpochResults)
        .function("getTrainingLoss", &WASMFitzHughNagumo::getTrainingLoss)
        .function("getValidationLoss", &WASMFitzHughNagumo::getValidationLoss)
        .function("getCurrentEpoch", &WASMFitzHughNagumo::getCurrentEpoch)
        .function("getTrainingStats", &WASMFitzHughNagumo::getTrainingStats)
        .function("startTraining", &WASMFitzHughNagumo::startTraining)
        .function("stopTraining", &WASMFitzHughNagumo::stopTraining)
        .function("getClassificationMetrics", &WASMFitzHughNagumo::getClassificationMetrics)
        .function("getConfusionMatrix", &WASMFitzHughNagumo::getConfusionMatrix)
        .function("getClassificationHistory", &WASMFitzHughNagumo::getClassificationHistory);
    
    // DTM bindings
    class_<WASMDTM>("DTM")
        .constructor<int, int, double>()
        .function("loadFromFile", &WASMDTM::loadFromFile)
        .function("saveToFile", &WASMDTM::saveToFile)
        .function("getElevation", &WASMDTM::getElevation)
        .function("setElevation", &WASMDTM::setElevation)
        .function("getDimensions", &WASMDTM::getDimensions)
        .function("getCellSize", &WASMDTM::getCellSize)
        .function("calculateSlope", &WASMDTM::calculateSlope)
        .function("calculateAspect", &WASMDTM::calculateAspect);
    
    // File processor bindings
    class_<FileProcessor>("FileProcessor")
        .class_function("processECGData", &FileProcessor::processECGData)
        .class_function("detectRPeaks", &FileProcessor::detectRPeaks)
        .class_function("calculateECGMetrics", &FileProcessor::calculateECGMetrics);
}

