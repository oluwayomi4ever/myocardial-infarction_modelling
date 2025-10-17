#include <iostream>
#include <cassert>
#include <cmath>
#include "DTM.h"
#include "FitzHughNagumo.h"
#include "ValidationFramework.h"

/**
 * @file simple_test_main.cpp
 * @brief Simple unit tests without external dependencies
 */

bool testDTMBasicFunctionality() {
    std::cout << "Testing DTM basic functionality..." << std::endl;
    
    try {
        // Create DTM instance
        DTM dtm(10, 10, 1.0);
        
        // Test basic operations
        dtm.setElevation(5, 5, 100.0);
        double elevation = dtm.getElevation(5, 5);
        
        if (std::abs(elevation - 100.0) > 1e-6) {
            std::cerr << "Error: DTM elevation setting/getting failed" << std::endl;
            return false;
        }
        
        // Test dimensions
        auto dims = dtm.getDimensions();
        if (dims.first != 10 || dims.second != 10) {
            std::cerr << "Error: DTM dimensions incorrect" << std::endl;
            return false;
        }
        
        // Test file I/O
        bool save_result = dtm.saveToFile("test_dtm.dat");
        if (!save_result) {
            std::cerr << "Error: DTM save failed" << std::endl;
            return false;
        }
        
        // Test loading
        DTM dtm2(10, 10);
        bool load_result = dtm2.loadFromFile("test_dtm.dat");
        if (!load_result) {
            std::cerr << "Error: DTM load failed" << std::endl;
            return false;
        }
        
        std::cout << "DTM tests passed!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "DTM test failed with exception: " << e.what() << std::endl;
        return false;
    }
}

bool testFitzHughNagumoBasicFunctionality() {
    std::cout << "Testing FitzHugh-Nagumo basic functionality..." << std::endl;
    
    try {
        // Create FitzHugh-Nagumo model
        FitzHughNagumo fhn(20, 20, 0.01);
        
        // Initialize
        fhn.initialize();
        
        // Test parameter setting
        fhn.setParameters(0.1, 0.5, 1.0, 0.0);
        fhn.setDiffusionCoefficients(0.1, 0.0);
        
        // Test stimulus
        fhn.addStimulus(10, 10, 1.0, 10.0);
        
        // Test single step
        fhn.step();
        
        // Test multiple steps
        fhn.run(10);
        
        // Test data access
        const auto& u_data = fhn.getU();
        const auto& v_data = fhn.getV();
        
        if (u_data.empty() || v_data.empty()) {
            std::cerr << "Error: FitzHugh-Nagumo data access failed" << std::endl;
            return false;
        }
        
        // Test state saving
        bool save_result = fhn.saveState("test_fhn.dat");
        if (!save_result) {
            std::cerr << "Error: FitzHugh-Nagumo save failed" << std::endl;
            return false;
        }
        
        std::cout << "FitzHugh-Nagumo tests passed!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "FitzHugh-Nagumo test failed with exception: " << e.what() << std::endl;
        return false;
    }
}

bool testValidationMetrics() {
    std::cout << "Testing validation metrics..." << std::endl;
    
    try {
        // Test data
        std::vector<double> predicted = {1.0, 2.0, 3.0, 4.0, 5.0};
        std::vector<double> observed = {1.1, 1.9, 3.1, 3.9, 5.1};
        
        // Test RMSE
        double rmse = ValidationMetrics::calculateRMSE(predicted, observed);
        if (rmse != rmse || rmse < 0.0) {  // Check for NaN using NaN != NaN property
            std::cerr << "Error: RMSE calculation failed" << std::endl;
            return false;
        }
        
        // Test MAE
        double mae = ValidationMetrics::calculateMAE(predicted, observed);
        if (mae != mae || mae < 0.0) {  // Check for NaN
            std::cerr << "Error: MAE calculation failed" << std::endl;
            return false;
        }
        
        // Test correlation
        double correlation = ValidationMetrics::calculateCorrelation(predicted, observed);
        if (correlation != correlation) {  // Check for NaN
            std::cerr << "Error: Correlation calculation failed" << std::endl;
            return false;
        }
        
        // Test R-squared
        double r_squared = ValidationMetrics::calculateRSquared(predicted, observed);
        if (r_squared != r_squared) {  // Check for NaN
            std::cerr << "Error: R-squared calculation failed" << std::endl;
            return false;
        }
        
        // Test normalized metrics
        auto normalized_metrics = ValidationMetrics::calculateNormalizedMetrics(predicted, observed);
        if (normalized_metrics.empty()) {
            std::cerr << "Error: Normalized metrics calculation failed" << std::endl;
            return false;
        }
        
        std::cout << "Validation metrics tests passed!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Validation metrics test failed with exception: " << e.what() << std::endl;
        return false;
    }
}

bool testModelValidator() {
    std::cout << "Testing model validator..." << std::endl;
    
    try {
        ModelValidator validator;
        
        // Create test validation data
        std::vector<std::vector<double>> validation_data(5, std::vector<double>(5, 1.0));
        validator.addValidationData("test_dataset", validation_data);
        
        // Create test model predictions
        std::vector<std::vector<double>> predictions(5, std::vector<double>(5, 1.1));
        
        // Test validation
        auto results = validator.validateModel(predictions, "test_dataset");
        if (results.empty()) {
            std::cerr << "Error: Model validation failed" << std::endl;
            return false;
        }
        
        // Test report generation
        bool report_result = validator.generateValidationReport("test_validation_report.txt");
        if (!report_result) {
            std::cerr << "Error: Validation report generation failed" << std::endl;
            return false;
        }
        
        std::cout << "Model validator tests passed!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Model validator test failed with exception: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "Running MI Modeling C++ Project Tests" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    int passed_tests = 0;
    int total_tests = 4;
    
    // Run tests
    if (testDTMBasicFunctionality()) {
        passed_tests++;
    }
    
    if (testFitzHughNagumoBasicFunctionality()) {
        passed_tests++;
    }
    
    if (testValidationMetrics()) {
        passed_tests++;
    }
    
    if (testModelValidator()) {
        passed_tests++;
    }
    
    // Print results
    std::cout << std::endl;
    std::cout << "Test Results: " << passed_tests << "/" << total_tests << " tests passed" << std::endl;
    
    if (passed_tests == total_tests) {
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests failed!" << std::endl;
        return 1;
    }
}
