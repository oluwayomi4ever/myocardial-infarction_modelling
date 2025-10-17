#ifndef VALIDATIONFRAMEWORK_H
#define VALIDATIONFRAMEWORK_H

/**
 * @file ValidationFramework.h
 * @brief Validation and comparison tools for MI model evaluation
 */

#include <vector>
#include <string>
#include <map>
#include <functional>

/**
 * @brief Statistical validation metrics
 */
class ValidationMetrics {
public:
    /**
     * @brief Calculate Root Mean Square Error (RMSE)
     * @param predicted Predicted values
     * @param observed Observed values
     * @return RMSE value
     */
    static double calculateRMSE(const std::vector<double>& predicted, 
                               const std::vector<double>& observed);
    
    /**
     * @brief Calculate Mean Absolute Error (MAE)
     * @param predicted Predicted values
     * @param observed Observed values
     * @return MAE value
     */
    static double calculateMAE(const std::vector<double>& predicted,
                              const std::vector<double>& observed);
    
    /**
     * @brief Calculate correlation coefficient
     * @param predicted Predicted values
     * @param observed Observed values
     * @return Correlation coefficient
     */
    static double calculateCorrelation(const std::vector<double>& predicted,
                                      const std::vector<double>& observed);
    
    /**
     * @brief Calculate R-squared
     * @param predicted Predicted values
     * @param observed Observed values
     * @return R-squared value
     */
    static double calculateRSquared(const std::vector<double>& predicted,
                                   const std::vector<double>& observed);
    
    /**
     * @brief Calculate normalized error metrics
     * @param predicted Predicted values
     * @param observed Observed values
     * @return Map of normalized metrics
     */
    static std::map<std::string, double> calculateNormalizedMetrics(
        const std::vector<double>& predicted,
        const std::vector<double>& observed);
};

/**
 * @brief Model validation framework
 */
class ModelValidator {
public:
    ModelValidator();
    ~ModelValidator();
    
    /**
     * @brief Add validation dataset
     * @param name Dataset name
     * @param data Validation data
     */
    void addValidationData(const std::string& name, 
                          const std::vector<std::vector<double>>& data);
    
    /**
     * @brief Validate model predictions
     * @param model_predictions Model output data
     * @param dataset_name Name of validation dataset
     * @return Validation results
     */
    std::map<std::string, double> validateModel(
        const std::vector<std::vector<double>>& model_predictions,
        const std::string& dataset_name);
    
    /**
     * @brief Perform cross-validation
     * @param model_func Model function to validate
     * @param k_folds Number of folds for cross-validation
     * @return Cross-validation results
     */
    std::map<std::string, double> performCrossValidation(
        std::function<std::vector<std::vector<double>>()> model_func,
        int k_folds = 5);
    
    /**
     * @brief Generate validation report
     * @param filename Output report filename
     * @return true if successful
     */
    bool generateValidationReport(const std::string& filename) const;

private:
    std::map<std::string, std::vector<std::vector<double>>> validation_datasets_;
    std::vector<std::map<std::string, double>> validation_results_;
    
    /**
     * @brief Split data into training and testing sets
     */
    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>
    splitData(const std::vector<std::vector<double>>& data, double test_ratio = 0.2);
};

/**
 * @brief Sensitivity analysis tools
 */
class SensitivityAnalyzer {
public:
    SensitivityAnalyzer();
    ~SensitivityAnalyzer();
    
    /**
     * @brief Perform parameter sensitivity analysis
     * @param base_parameters Base parameter values
     * @param parameter_ranges Parameter variation ranges
     * @param model_func Model evaluation function
     * @return Sensitivity indices
     */
    std::map<std::string, double> analyzeParameterSensitivity(
        const std::map<std::string, double>& base_parameters,
        const std::map<std::string, std::pair<double, double>>& parameter_ranges,
        std::function<double(const std::map<std::string, double>&)> model_func);
    
    /**
     * @brief Perform global sensitivity analysis using Sobol indices
     * @param parameters Parameter names
     * @param model_func Model evaluation function
     * @param n_samples Number of samples
     * @return Sobol sensitivity indices
     */
    std::map<std::string, double> calculateSobolIndices(
        const std::vector<std::string>& parameters,
        std::function<double(const std::map<std::string, double>&)> model_func,
        int n_samples = 1000);

private:
    /**
     * @brief Generate parameter samples using Latin Hypercube Sampling
     */
    std::vector<std::map<std::string, double>> generateLHSSamples(
        const std::map<std::string, std::pair<double, double>>& parameter_ranges,
        int n_samples);
};

/**
 * @brief Clinical data comparison tools
 */
class ClinicalDataComparator {
public:
    ClinicalDataComparator();
    ~ClinicalDataComparator();
    
    /**
     * @brief Compare model output with clinical measurements
     * @param model_output Model simulation results
     * @param clinical_data Clinical measurement data
     * @param measurement_type Type of measurement (ECG, MRI, Echo)
     * @return Comparison results
     */
    std::map<std::string, double> compareWithClinicalData(
        const std::vector<std::vector<double>>& model_output,
        const std::vector<std::vector<double>>& clinical_data,
        const std::string& measurement_type);
    
    /**
     * @brief Validate ECG parameters
     * @param model_ecg Model-generated ECG
     * @param clinical_ecg Clinical ECG data
     * @return ECG validation metrics
     */
    std::map<std::string, double> validateECGParameters(
        const std::vector<double>& model_ecg,
        const std::vector<double>& clinical_ecg);
    
    /**
     * @brief Validate cardiac mechanics
     * @param model_mechanics Model mechanics output
     * @param clinical_mechanics Clinical mechanics data
     * @return Mechanics validation metrics
     */
    std::map<std::string, double> validateCardiacMechanics(
        const std::vector<std::vector<double>>& model_mechanics,
        const std::vector<std::vector<double>>& clinical_mechanics);
    
    /**
     * @brief Generate clinical comparison report
     * @param filename Output report filename
     * @return true if successful
     */
    bool generateClinicalReport(const std::string& filename) const;

private:
    std::vector<std::map<std::string, double>> comparison_results_;
    
    /**
     * @brief Extract ECG features
     */
    std::map<std::string, double> extractECGFeatures(const std::vector<double>& ecg_signal);
    
    /**
     * @brief Extract mechanics features
     */
    std::map<std::string, double> extractMechanicsFeatures(
        const std::vector<std::vector<double>>& mechanics_data);
};

/**
 * @brief Uncertainty quantification tools
 */
class UncertaintyQuantifier {
public:
    UncertaintyQuantifier();
    ~UncertaintyQuantifier();
    
    /**
     * @brief Perform Monte Carlo uncertainty analysis
     * @param model_func Model evaluation function
     * @param parameter_distributions Parameter probability distributions
     * @param n_samples Number of Monte Carlo samples
     * @return Uncertainty statistics
     */
    std::map<std::string, double> performMonteCarloAnalysis(
        std::function<std::vector<std::vector<double>>()> model_func,
        const std::map<std::string, std::function<double()>>& parameter_distributions,
        int n_samples = 1000);
    
    /**
     * @brief Calculate prediction intervals
     * @param model_outputs Multiple model outputs
     * @param confidence_level Confidence level (0-1)
     * @return Prediction intervals
     */
    std::map<std::string, std::pair<double, double>> calculatePredictionIntervals(
        const std::vector<std::vector<std::vector<double>>>& model_outputs,
        double confidence_level = 0.95);

private:
    /**
     * @brief Calculate statistics from ensemble
     */
    std::map<std::string, double> calculateEnsembleStatistics(
        const std::vector<std::vector<std::vector<double>>>& outputs);
};

#endif // VALIDATIONFRAMEWORK_H

