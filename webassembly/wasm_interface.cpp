#include <emscripten.h>
#include <emscripten/bind.h>
#include <vector>
#include <string>
#include <memory>

// Include your existing MI modeling classes
#include "../include/FitzHughNagumo.h"
#include "../include/DTM.h"

using namespace emscripten;

// WebAssembly wrapper for FitzHugh-Nagumo model
class WASMFitzHughNagumo {
private:
    std::unique_ptr<FitzHughNagumo> model;
    
public:
    WASMFitzHughNagumo(int width, int height, double dt = 0.01) {
        model = std::make_unique<FitzHughNagumo>(width, height, dt);
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
    
    void run(int steps) {
        model->run(steps);
    }
    
    double getTime() const {
        return model->getTime();
    }
    
    // Return membrane potential data as vector for JavaScript
    std::vector<std::vector<double>> getMembranePotential() const {
        return model->getU();
    }
    
    // Return recovery variable data as vector for JavaScript
    std::vector<std::vector<double>> getRecoveryVariable() const {
        return model->getV();
    }
    
    bool saveState(const std::string& filename) const {
        return model->saveState(filename);
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
    
    std::vector<int> getDimensions() const {
        auto dims = dtm->getDimensions();
        return {dims.first, dims.second};
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

// Global simulation runner function
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
    int process_ecg_data(double* input_data, int length, double* output_data) {
        // Process ECG data and return processed length
        for (int i = 0; i < length; i++) {
            // Simple processing example - apply low-pass filter
            output_data[i] = input_data[i] * 0.8; // Simplified processing
        }
        return length;
    }
}

// Emscripten bindings to expose classes to JavaScript
EMSCRIPTEN_BINDINGS(mi_modeling) {
    // FitzHugh-Nagumo bindings
    class_<WASMFitzHughNagumo>("FitzHughNagumo")
        .constructor<int, int, double>()
        .function("initialize", &WASMFitzHughNagumo::initialize)
        .function("setParameters", &WASMFitzHughNagumo::setParameters)
        .function("setDiffusionCoefficients", &WASMFitzHughNagumo::setDiffusionCoefficients)
        .function("run", &WASMFitzHughNagumo::run)
        .function("getTime", &WASMFitzHughNagumo::getTime)
        .function("getMembranePotential", &WASMFitzHughNagumo::getMembranePotential)
        .function("getRecoveryVariable", &WASMFitzHughNagumo::getRecoveryVariable)
        .function("saveState", &WASMFitzHughNagumo::saveState);
    
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
    
    // Vector bindings for data transfer
    register_vector<double>("vector<double>");
    register_vector<std::vector<double>>("vector<vector<double>>");
    register_vector<int>("vector<int>");
}



