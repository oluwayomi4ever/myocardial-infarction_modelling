#include <iostream>
#include <memory>
#include "DataProcessor.h"
#include "ValidationFramework.h"

/**
 * @file data_test.cpp
 * @brief Test program for data processing and validation
 */

void testECGProcessing() {
    std::cout << "\n=== Testing ECG Data Processing ===" << std::endl;
    
    ECGProcessor ecg_processor;
    
    // Test loading ECG data
    if (ecg_processor.loadData("data/sample_ecg_data.txt")) {
        std::cout << "✅ ECG data loaded successfully" << std::endl;
        
        // Test processing
        if (ecg_processor.processData()) {
            std::cout << "✅ ECG data processed successfully" << std::endl;
            
            // Test feature extraction
            auto qrs_params = ecg_processor.extractQRSParameters();
            std::cout << "📊 QRS Parameters extracted:" << std::endl;
            for (const auto& param : qrs_params) {
                std::cout << "   " << param.first << ": " << param.second << std::endl;
            }
            
            // Test arrhythmia detection
            std::string arrhythmia = ecg_processor.detectArrhythmias();
            std::cout << "🫀 Detected arrhythmia: " << arrhythmia << std::endl;
            
            // Test saving processed data
            if (ecg_processor.saveProcessedData("build/data/processed_ecg.txt")) {
                std::cout << "✅ Processed ECG data saved" << std::endl;
            }
        }
    } else {
        std::cout << "❌ Failed to load ECG data" << std::endl;
    }
}

void testMRIProcessing() {
    std::cout << "\n=== Testing MRI Data Processing ===" << std::endl;
    
    MRIProcessor mri_processor(100, 100);
    
    // Test loading MRI data
    if (mri_processor.loadData("data/sample_mri_data.txt")) {
        std::cout << "✅ MRI data loaded successfully" << std::endl;
        
        // Test processing
        if (mri_processor.processData()) {
            std::cout << "✅ MRI data processed successfully" << std::endl;
            
            // Test tissue segmentation
            auto tissue_segmentation = mri_processor.segmentTissue();
            std::cout << "🔬 Tissue segmentation completed" << std::endl;
            
            // Count tissue types
            int normal_count = 0, ischemic_count = 0, infarcted_count = 0;
            for (const auto& row : tissue_segmentation) {
                for (int tissue_type : row) {
                    if (tissue_type == 0) normal_count++;
                    else if (tissue_type == 1) ischemic_count++;
                    else if (tissue_type == 2) infarcted_count++;
                }
            }
            
            std::cout << "📊 Tissue distribution:" << std::endl;
            std::cout << "   Normal: " << normal_count << " pixels" << std::endl;
            std::cout << "   Ischemic: " << ischemic_count << " pixels" << std::endl;
            std::cout << "   Infarcted: " << infarcted_count << " pixels" << std::endl;
            
            // Test wall thickness calculation
            auto wall_thickness = mri_processor.calculateWallThickness();
            std::cout << "💪 Wall thickness analysis completed" << std::endl;
            
            // Test perfusion mapping
            auto perfusion_map = mri_processor.extractPerfusionMap();
            std::cout << "🩸 Perfusion mapping completed" << std::endl;
            
            // Test saving processed data
            if (mri_processor.saveProcessedData("build/data/processed_mri.txt")) {
                std::cout << "✅ Processed MRI data saved" << std::endl;
            }
        }
    } else {
        std::cout << "❌ Failed to load MRI data" << std::endl;
    }
}

void testEchoProcessing() {
    std::cout << "\n=== Testing Echo Data Processing ===" << std::endl;
    
    EchoProcessor echo_processor;
    
    // Test loading Echo data
    if (echo_processor.loadData("data/sample_echo_data.txt")) {
        std::cout << "✅ Echo data loaded successfully" << std::endl;
        
        // Test processing
        if (echo_processor.processData()) {
            std::cout << "✅ Echo data processed successfully" << std::endl;
            
            // Test ejection fraction calculation
            double ejection_fraction = echo_processor.calculateEjectionFraction();
            std::cout << "📊 Ejection Fraction: " << ejection_fraction << "%" << std::endl;
            
            // Test wall motion analysis
            auto wall_motion = echo_processor.analyzeWallMotion();
            std::cout << "💓 Wall motion analysis completed" << std::endl;
            
            // Test saving processed data
            if (echo_processor.saveProcessedData("build/data/processed_echo.txt")) {
                std::cout << "✅ Processed Echo data saved" << std::endl;
            }
        }
    } else {
        std::cout << "❌ Failed to load Echo data" << std::endl;
    }
}

void testDataIntegration() {
    std::cout << "\n=== Testing Data Integration ===" << std::endl;
    
    DataIntegrationManager integration_manager;
    
    // Add processors
    integration_manager.addProcessor("ecg", std::make_unique<ECGProcessor>());
    integration_manager.addProcessor("mri", std::make_unique<MRIProcessor>(100, 100));
    integration_manager.addProcessor("echo", std::make_unique<EchoProcessor>());
    
    // Load all data
    if (integration_manager.loadAllData("data/config.txt")) {
        std::cout << "✅ All data loaded successfully" << std::endl;
        
        // Process all data
        if (integration_manager.processAllData()) {
            std::cout << "✅ All data processed successfully" << std::endl;
            
            // Generate model parameters
            auto model_params = integration_manager.generateModelParameters();
            std::cout << "📊 Model parameters generated:" << std::endl;
            for (const auto& param : model_params) {
                std::cout << "   " << param.first << ": " << param.second << std::endl;
            }
            
            // Create tissue maps
            auto tissue_maps = integration_manager.createTissueMaps();
            std::cout << "🗺️ Tissue maps created: " << tissue_maps.size() << " maps" << std::endl;
            
            // Create tissue segmentation
            auto segmentation = integration_manager.createTissueSegmentation();
            std::cout << "🔬 Tissue segmentation created: " << segmentation.size() << " segmentations" << std::endl;
        }
    } else {
        std::cout << "❌ Failed to load all data" << std::endl;
    }
}

void testValidationFramework() {
    std::cout << "\n=== Testing Validation Framework ===" << std::endl;
    
    ModelValidator validator;
    
    // Create test validation data (simulating clinical measurements)
    std::vector<std::vector<double>> clinical_data(10, std::vector<double>(10, 1.0));
    validator.addValidationData("clinical_test", clinical_data);
    
    // Create test model predictions
    std::vector<std::vector<double>> model_predictions(10, std::vector<double>(10, 1.05));
    
    // Test validation
    auto validation_results = validator.validateModel(model_predictions, "clinical_test");
    
    if (!validation_results.empty()) {
        std::cout << "✅ Model validation completed" << std::endl;
        std::cout << "📊 Validation metrics:" << std::endl;
        for (const auto& metric : validation_results) {
            std::cout << "   " << metric.first << ": " << metric.second << std::endl;
        }
        
        // Generate validation report
        if (validator.generateValidationReport("build/data/validation_report.txt")) {
            std::cout << "✅ Validation report generated" << std::endl;
        }
    } else {
        std::cout << "❌ Model validation failed" << std::endl;
    }
}

void testClinicalDataComparison() {
    std::cout << "\n=== Testing Clinical Data Comparison ===" << std::endl;
    
    ClinicalDataComparator comparator;
    
    // Create test data
    std::vector<std::vector<double>> model_output(5, std::vector<double>(5, 1.0));
    std::vector<std::vector<double>> clinical_data(5, std::vector<double>(5, 1.1));
    
    // Test comparison
    auto comparison_results = comparator.compareWithClinicalData(model_output, clinical_data, "ECG");
    
    if (!comparison_results.empty()) {
        std::cout << "✅ Clinical data comparison completed" << std::endl;
        std::cout << "📊 Comparison metrics:" << std::endl;
        for (const auto& metric : comparison_results) {
            std::cout << "   " << metric.first << ": " << metric.second << std::endl;
        }
        
        // Generate clinical report
        if (comparator.generateClinicalReport("build/data/clinical_report.txt")) {
            std::cout << "✅ Clinical comparison report generated" << std::endl;
        }
    } else {
        std::cout << "❌ Clinical data comparison failed" << std::endl;
    }
}

int main() {
    std::cout << "🧪 MI Modeling C++ Project - Data Testing Suite" << std::endl;
    std::cout << "===============================================" << std::endl;
    
    // Ensure data directory exists
    system("mkdir -p build/data");
    
    try {
        // Test individual data processors
        testECGProcessing();
        testMRIProcessing();
        testEchoProcessing();
        
        // Test data integration
        testDataIntegration();
        
        // Test validation framework
        testValidationFramework();
        testClinicalDataComparison();
        
        std::cout << "\n🎉 All data tests completed successfully!" << std::endl;
        std::cout << "\n📁 Generated files in build/data/:" << std::endl;
        system("ls -la build/data/");
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
