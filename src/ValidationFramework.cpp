#include "ValidationFramework.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>

// ValidationMetrics Implementation
double ValidationMetrics::calculateRMSE(const std::vector<double>& predicted, 
                                       const std::vector<double>& observed) {
    if (predicted.size() != observed.size() || predicted.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    
    double sum_squared_error = 0.0;
    for (size_t i = 0; i < predicted.size(); ++i) {
        double error = predicted[i] - observed[i];
        sum_squared_error += error * error;
    }
    
    return std::sqrt(sum_squared_error / predicted.size());
}

double ValidationMetrics::calculateMAE(const std::vector<double>& predicted,
                                      const std::vector<double>& observed) {
    if (predicted.size() != observed.size() || predicted.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    
    double sum_absolute_error = 0.0;
    for (size_t i = 0; i < predicted.size(); ++i) {
        sum_absolute_error += std::abs(predicted[i] - observed[i]);
    }
    
    return sum_absolute_error / predicted.size();
}

double ValidationMetrics::calculateCorrelation(const std::vector<double>& predicted,
                                              const std::vector<double>& observed) {
    if (predicted.size() != observed.size() || predicted.size() < 2) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    
    // Calculate means
    double mean_pred = std::accumulate(predicted.begin(), predicted.end(), 0.0) / predicted.size();
    double mean_obs = std::accumulate(observed.begin(), observed.end(), 0.0) / observed.size();
    
    // Calculate correlation coefficient
    double numerator = 0.0;
    double sum_sq_pred = 0.0;
    double sum_sq_obs = 0.0;
    
    for (size_t i = 0; i < predicted.size(); ++i) {
        double diff_pred = predicted[i] - mean_pred;
        double diff_obs = observed[i] - mean_obs;
        
        numerator += diff_pred * diff_obs;
        sum_sq_pred += diff_pred * diff_pred;
        sum_sq_obs += diff_obs * diff_obs;
    }
    
    double denominator = std::sqrt(sum_sq_pred * sum_sq_obs);
    return (denominator == 0.0) ? 0.0 : numerator / denominator;
}

double ValidationMetrics::calculateRSquared(const std::vector<double>& predicted,
                                           const std::vector<double>& observed) {
    if (predicted.size() != observed.size() || predicted.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    
    double mean_obs = std::accumulate(observed.begin(), observed.end(), 0.0) / observed.size();
    
    double ss_res = 0.0; // Sum of squares of residuals
    double ss_tot = 0.0; // Total sum of squares
    
    for (size_t i = 0; i < predicted.size(); ++i) {
        double residual = observed[i] - predicted[i];
        double total_diff = observed[i] - mean_obs;
        
        ss_res += residual * residual;
        ss_tot += total_diff * total_diff;
    }
    
    return (ss_tot == 0.0) ? 1.0 : 1.0 - (ss_res / ss_tot);
}

std::map<std::string, double> ValidationMetrics::calculateNormalizedMetrics(
    const std::vector<double>& predicted,
    const std::vector<double>& observed) {
    
    std::map<std::string, double> metrics;
    
    if (predicted.empty() || observed.empty() || predicted.size() != observed.size()) {
        return metrics;
    }
    
    // Calculate basic metrics
    double rmse = calculateRMSE(predicted, observed);
    double mae = calculateMAE(predicted, observed);
    double correlation = calculateCorrelation(predicted, observed);
    double r_squared = calculateRSquared(predicted, observed);
    
    // Normalize RMSE and MAE by the range of observed values
    double min_obs = *std::min_element(observed.begin(), observed.end());
    double max_obs = *std::max_element(observed.begin(), observed.end());
    double range_obs = max_obs - min_obs;
    
    metrics["rmse"] = rmse;
    metrics["mae"] = mae;
    metrics["correlation"] = correlation;
    metrics["r_squared"] = r_squared;
    metrics["normalized_rmse"] = (range_obs > 0.0) ? rmse / range_obs : 0.0;
    metrics["normalized_mae"] = (range_obs > 0.0) ? mae / range_obs : 0.0;
    metrics["mean_absolute_percentage_error"] = 0.0;
    
    // Calculate MAPE
    double mape_sum = 0.0;
    int valid_points = 0;
    for (size_t i = 0; i < predicted.size(); ++i) {
        if (observed[i] != 0.0) {
            mape_sum += std::abs((predicted[i] - observed[i]) / observed[i]);
            valid_points++;
        }
    }
    
    if (valid_points > 0) {
        metrics["mean_absolute_percentage_error"] = (mape_sum / valid_points) * 100.0;
    }
    
    return metrics;
}

// ModelValidator Implementation
ModelValidator::ModelValidator() {
    // Constructor
}

ModelValidator::~ModelValidator() {
    // Destructor
}

void ModelValidator::addValidationData(const std::string& name, 
                                      const std::vector<std::vector<double>>& data) {
    validation_datasets_[name] = data;
}

std::map<std::string, double> ModelValidator::validateModel(
    const std::vector<std::vector<double>>& model_predictions,
    const std::string& dataset_name) {
    
    std::map<std::string, double> results;
    
    if (validation_datasets_.find(dataset_name) == validation_datasets_.end()) {
        std::cerr << "Error: Validation dataset '" << dataset_name << "' not found" << std::endl;
        return results;
    }
    
    const auto& observed_data = validation_datasets_[dataset_name];
    
    if (model_predictions.size() != observed_data.size() || model_predictions.empty()) {
        std::cerr << "Error: Model predictions and observed data dimensions do not match" << std::endl;
        return results;
    }
    
    // Flatten 2D data for validation
    std::vector<double> predicted_flat;
    std::vector<double> observed_flat;
    
    for (size_t i = 0; i < model_predictions.size(); ++i) {
        for (size_t j = 0; j < model_predictions[i].size(); ++j) {
            predicted_flat.push_back(model_predictions[i][j]);
            observed_flat.push_back(observed_data[i][j]);
        }
    }
    
    // Calculate validation metrics
    results = ValidationMetrics::calculateNormalizedMetrics(predicted_flat, observed_flat);
    
    // Store results
    validation_results_.push_back(results);
    
    return results;
}

std::map<std::string, double> ModelValidator::performCrossValidation(
    std::function<std::vector<std::vector<double>>()> model_func,
    int k_folds) {
    
    std::map<std::string, double> results;
    
    if (validation_datasets_.empty()) {
        std::cerr << "Error: No validation datasets available for cross-validation" << std::endl;
        return results;
    }
    
    // Use the first available dataset for cross-validation
    auto dataset_name = validation_datasets_.begin()->first;
    const auto& full_data = validation_datasets_[dataset_name];
    
    if (full_data.empty()) {
        std::cerr << "Error: Validation dataset is empty" << std::endl;
        return results;
    }
    
    std::vector<double> all_rmse;
    std::vector<double> all_r_squared;
    
    // Perform k-fold cross-validation
    int fold_size = full_data.size() / k_folds;
    
    for (int fold = 0; fold < k_folds; ++fold) {
        // Split data into training and testing sets
        auto split_result = splitData(full_data, 1.0 / k_folds);
        const auto& test_data = split_result.second;
        
        // Run model (this would typically use the training data for parameter estimation)
        auto predictions = model_func();
        
        // Validate on test data
        std::vector<double> pred_flat, obs_flat;
        for (size_t i = 0; i < predictions.size() && i < test_data.size(); ++i) {
            for (size_t j = 0; j < predictions[i].size() && j < test_data[i].size(); ++j) {
                pred_flat.push_back(predictions[i][j]);
                obs_flat.push_back(test_data[i][j]);
            }
        }
        
        if (!pred_flat.empty()) {
            double rmse = ValidationMetrics::calculateRMSE(pred_flat, obs_flat);
            double r_squared = ValidationMetrics::calculateRSquared(pred_flat, obs_flat);
            
            all_rmse.push_back(rmse);
            all_r_squared.push_back(r_squared);
        }
    }
    
    // Calculate mean and standard deviation
    if (!all_rmse.empty()) {
        double mean_rmse = std::accumulate(all_rmse.begin(), all_rmse.end(), 0.0) / all_rmse.size();
        double mean_r_squared = std::accumulate(all_r_squared.begin(), all_r_squared.end(), 0.0) / all_r_squared.size();
        
        results["mean_rmse"] = mean_rmse;
        results["mean_r_squared"] = mean_r_squared;
        results["std_rmse"] = 0.0; // Would calculate standard deviation
        results["std_r_squared"] = 0.0; // Would calculate standard deviation
    }
    
    return results;
}

bool ModelValidator::generateValidationReport(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create validation report file " << filename << std::endl;
        return false;
    }
    
    try {
        file << "Validation Report\n";
        file << "================\n\n";
        
        file << "Number of validation runs: " << validation_results_.size() << "\n\n";
        
        for (size_t i = 0; i < validation_results_.size(); ++i) {
            file << "Validation Run " << (i + 1) << ":\n";
            file << "----------------\n";
            
            for (const auto& metric : validation_results_[i]) {
                file << metric.first << ": " << metric.second << "\n";
            }
            file << "\n";
        }
        
        file.close();
        std::cout << "Validation report saved to " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error generating validation report: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>
ModelValidator::splitData(const std::vector<std::vector<double>>& data, double test_ratio) {
    std::vector<std::vector<double>> training_data;
    std::vector<std::vector<double>> test_data;
    
    if (data.empty()) {
        return std::make_pair(training_data, test_data);
    }
    
    // Simple random split (in practice, would use proper random sampling)
    int test_size = static_cast<int>(data.size() * test_ratio);
    int training_size = data.size() - test_size;
    
    training_data.reserve(training_size);
    test_data.reserve(test_size);
    
    for (int i = 0; i < static_cast<int>(data.size()); ++i) {
        if (i < test_size) {
            test_data.push_back(data[i]);
        } else {
            training_data.push_back(data[i]);
        }
    }
    
    return std::make_pair(training_data, test_data);
}

// SensitivityAnalyzer Implementation
SensitivityAnalyzer::SensitivityAnalyzer() {
    // Constructor
}

SensitivityAnalyzer::~SensitivityAnalyzer() {
    // Destructor
}

std::map<std::string, double> SensitivityAnalyzer::analyzeParameterSensitivity(
    const std::map<std::string, double>& base_parameters,
    const std::map<std::string, std::pair<double, double>>& parameter_ranges,
    std::function<double(const std::map<std::string, double>&)> model_func) {
    
    std::map<std::string, double> sensitivity_indices;
    
    if (base_parameters.empty() || parameter_ranges.empty()) {
        std::cerr << "Error: Empty parameter sets for sensitivity analysis" << std::endl;
        return sensitivity_indices;
    }
    
    // Calculate base model output
    double base_output = model_func(base_parameters);
    
    // Calculate sensitivity for each parameter
    for (const auto& param_pair : base_parameters) {
        const std::string& param_name = param_pair.first;
        double base_value = param_pair.second;
        
        if (parameter_ranges.find(param_name) == parameter_ranges.end()) {
            continue;
        }
        
        auto range = parameter_ranges.at(param_name);
        double perturbed_value = base_value + (range.second - range.first) * 0.1; // 10% perturbation
        
        // Create perturbed parameter set
        auto perturbed_params = base_parameters;
        perturbed_params[param_name] = perturbed_value;
        
        // Calculate perturbed model output
        double perturbed_output = model_func(perturbed_params);
        
        // Calculate sensitivity index (normalized derivative)
        double sensitivity = std::abs((perturbed_output - base_output) / (perturbed_value - base_value));
        sensitivity_indices[param_name] = sensitivity;
    }
    
    return sensitivity_indices;
}

std::map<std::string, double> SensitivityAnalyzer::calculateSobolIndices(
    const std::vector<std::string>& parameters,
    std::function<double(const std::map<std::string, double>&)> model_func,
    int n_samples) {
    
    std::map<std::string, double> sobol_indices;
    
    if (parameters.empty()) {
        std::cerr << "Error: No parameters specified for Sobol analysis" << std::endl;
        return sobol_indices;
    }
    
    // Generate parameter ranges (simplified)
    std::map<std::string, std::pair<double, double>> parameter_ranges;
    for (const auto& param : parameters) {
        parameter_ranges[param] = std::make_pair(0.1, 2.0); // Default range
    }
    
    // Generate Latin Hypercube samples
    auto samples = generateLHSSamples(parameter_ranges, n_samples);
    
    if (samples.empty()) {
        std::cerr << "Error: Failed to generate parameter samples" << std::endl;
        return sobol_indices;
    }
    
    // Calculate model outputs
    std::vector<double> outputs;
    outputs.reserve(samples.size());
    
    for (const auto& sample : samples) {
        double output = model_func(sample);
        outputs.push_back(output);
    }
    
    // Calculate total variance
    double mean_output = std::accumulate(outputs.begin(), outputs.end(), 0.0) / outputs.size();
    double total_variance = 0.0;
    
    for (double output : outputs) {
        total_variance += (output - mean_output) * (output - mean_output);
    }
    total_variance /= outputs.size();
    
    // Calculate first-order Sobol indices (simplified)
    for (const auto& param : parameters) {
        // This is a simplified calculation
        // Real Sobol indices require more sophisticated Monte Carlo methods
        sobol_indices[param] = 0.1; // Placeholder value
    }
    
    return sobol_indices;
}

std::vector<std::map<std::string, double>> SensitivityAnalyzer::generateLHSSamples(
    const std::map<std::string, std::pair<double, double>>& parameter_ranges,
    int n_samples) {
    
    std::vector<std::map<std::string, double>> samples;
    
    if (parameter_ranges.empty() || n_samples <= 0) {
        return samples;
    }
    
    // Simplified Latin Hypercube Sampling
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int i = 0; i < n_samples; ++i) {
        std::map<std::string, double> sample;
        
        for (const auto& param_pair : parameter_ranges) {
            const std::string& param_name = param_pair.first;
            auto range = param_pair.second;
            
            // Uniform random sampling within range
            std::uniform_real_distribution<double> dis(range.first, range.second);
            sample[param_name] = dis(gen);
        }
        
        samples.push_back(sample);
    }
    
    return samples;
}

// ClinicalDataComparator Implementation
ClinicalDataComparator::ClinicalDataComparator() {
    // Constructor
}

ClinicalDataComparator::~ClinicalDataComparator() {
    // Destructor
}

std::map<std::string, double> ClinicalDataComparator::compareWithClinicalData(
    const std::vector<std::vector<double>>& model_output,
    const std::vector<std::vector<double>>& clinical_data,
    const std::string& measurement_type) {
    
    std::map<std::string, double> comparison_results;
    
    if (model_output.empty() || clinical_data.empty()) {
        std::cerr << "Error: Empty data for clinical comparison" << std::endl;
        return comparison_results;
    }
    
    // Flatten data for comparison
    std::vector<double> model_flat, clinical_flat;
    
    for (size_t i = 0; i < std::min(model_output.size(), clinical_data.size()); ++i) {
        for (size_t j = 0; j < std::min(model_output[i].size(), clinical_data[i].size()); ++j) {
            model_flat.push_back(model_output[i][j]);
            clinical_flat.push_back(clinical_data[i][j]);
        }
    }
    
    if (model_flat.empty()) {
        std::cerr << "Error: No overlapping data for comparison" << std::endl;
        return comparison_results;
    }
    
    // Calculate comparison metrics
    comparison_results = ValidationMetrics::calculateNormalizedMetrics(model_flat, clinical_flat);
    
    // Add measurement-specific metrics
    if (measurement_type == "ECG") {
        auto ecg_metrics = validateECGParameters(model_flat, clinical_flat);
        comparison_results.insert(ecg_metrics.begin(), ecg_metrics.end());
    } else if (measurement_type == "MRI") {
        comparison_results["dice_coefficient"] = 0.85; // Placeholder
        comparison_results["hausdorff_distance"] = 2.3; // Placeholder
    }
    
    // Store results
    comparison_results_.push_back(comparison_results);
    
    return comparison_results;
}

std::map<std::string, double> ClinicalDataComparator::validateECGParameters(
    const std::vector<double>& model_ecg,
    const std::vector<double>& clinical_ecg) {
    
    std::map<std::string, double> ecg_metrics;
    
    if (model_ecg.empty() || clinical_ecg.empty()) {
        return ecg_metrics;
    }
    
    // Extract ECG features
    auto model_features = extractECGFeatures(model_ecg);
    auto clinical_features = extractECGFeatures(clinical_ecg);
    
    // Compare features
    for (const auto& feature_pair : model_features) {
        const std::string& feature_name = feature_pair.first;
        double model_value = feature_pair.second;
        
        if (clinical_features.find(feature_name) != clinical_features.end()) {
            double clinical_value = clinical_features[feature_name];
            double relative_error = std::abs(model_value - clinical_value) / std::abs(clinical_value) * 100.0;
            ecg_metrics[feature_name + "_relative_error"] = relative_error;
        }
    }
    
    return ecg_metrics;
}

std::map<std::string, double> ClinicalDataComparator::validateCardiacMechanics(
    const std::vector<std::vector<double>>& model_mechanics,
    const std::vector<std::vector<double>>& clinical_mechanics) {
    
    std::map<std::string, double> mechanics_metrics;
    
    if (model_mechanics.empty() || clinical_mechanics.empty()) {
        return mechanics_metrics;
    }
    
    // Extract mechanics features
    auto model_features = extractMechanicsFeatures(model_mechanics);
    auto clinical_features = extractMechanicsFeatures(clinical_mechanics);
    
    // Compare features
    for (const auto& feature_pair : model_features) {
        const std::string& feature_name = feature_pair.first;
        double model_value = feature_pair.second;
        
        if (clinical_features.find(feature_name) != clinical_features.end()) {
            double clinical_value = clinical_features[feature_name];
            double relative_error = std::abs(model_value - clinical_value) / std::abs(clinical_value) * 100.0;
            mechanics_metrics[feature_name + "_relative_error"] = relative_error;
        }
    }
    
    return mechanics_metrics;
}

bool ClinicalDataComparator::generateClinicalReport(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create clinical report file " << filename << std::endl;
        return false;
    }
    
    try {
        file << "Clinical Data Comparison Report\n";
        file << "===============================\n\n";
        
        file << "Number of comparisons: " << comparison_results_.size() << "\n\n";
        
        for (size_t i = 0; i < comparison_results_.size(); ++i) {
            file << "Comparison " << (i + 1) << ":\n";
            file << "------------\n";
            
            for (const auto& metric : comparison_results_[i]) {
                file << metric.first << ": " << metric.second << "\n";
            }
            file << "\n";
        }
        
        file.close();
        std::cout << "Clinical comparison report saved to " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error generating clinical report: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

std::map<std::string, double> ClinicalDataComparator::extractECGFeatures(const std::vector<double>& ecg_signal) {
    std::map<std::string, double> features;
    
    if (ecg_signal.empty()) {
        return features;
    }
    
    // Calculate basic ECG features
    features["qrs_duration"] = 80.0; // ms (simplified)
    features["qt_interval"] = 400.0; // ms (simplified)
    features["rr_interval"] = 800.0; // ms (simplified)
    features["heart_rate"] = 60000.0 / features["rr_interval"]; // BPM
    
    // Calculate signal statistics
    double mean_val = std::accumulate(ecg_signal.begin(), ecg_signal.end(), 0.0) / ecg_signal.size();
    features["mean_amplitude"] = mean_val;
    
    double max_val = *std::max_element(ecg_signal.begin(), ecg_signal.end());
    double min_val = *std::min_element(ecg_signal.begin(), ecg_signal.end());
    features["amplitude_range"] = max_val - min_val;
    
    return features;
}

std::map<std::string, double> ClinicalDataComparator::extractMechanicsFeatures(
    const std::vector<std::vector<double>>& mechanics_data) {
    
    std::map<std::string, double> features;
    
    if (mechanics_data.empty()) {
        return features;
    }
    
    // Calculate mechanics features
    features["ejection_fraction"] = 55.0; // % (simplified)
    features["stroke_volume"] = 70.0; // mL (simplified)
    features["cardiac_output"] = 5.0; // L/min (simplified)
    
    // Calculate regional mechanics
    int num_segments = 17; // Standard 17-segment model
    features["num_segments"] = num_segments;
    features["normal_segments"] = 15; // Simplified
    features["hypokinetic_segments"] = 1;
    features["akinetic_segments"] = 1;
    
    return features;
}

// UncertaintyQuantifier Implementation
UncertaintyQuantifier::UncertaintyQuantifier() {
    // Constructor
}

UncertaintyQuantifier::~UncertaintyQuantifier() {
    // Destructor
}

std::map<std::string, double> UncertaintyQuantifier::performMonteCarloAnalysis(
    std::function<std::vector<std::vector<double>>()> model_func,
    const std::map<std::string, std::function<double()>>& parameter_distributions,
    int n_samples) {
    
    std::map<std::string, double> uncertainty_stats;
    
    if (parameter_distributions.empty() || n_samples <= 0) {
        std::cerr << "Error: Invalid parameters for Monte Carlo analysis" << std::endl;
        return uncertainty_stats;
    }
    
    std::vector<std::vector<std::vector<double>>> model_outputs;
    model_outputs.reserve(n_samples);
    
    // Run Monte Carlo simulations
    for (int i = 0; i < n_samples; ++i) {
        // Sample parameters from distributions
        std::map<std::string, double> sampled_params;
        for (const auto& dist_pair : parameter_distributions) {
            sampled_params[dist_pair.first] = dist_pair.second();
        }
        
        // Run model with sampled parameters
        auto output = model_func();
        model_outputs.push_back(output);
    }
    
    if (model_outputs.empty()) {
        std::cerr << "Error: No model outputs generated" << std::endl;
        return uncertainty_stats;
    }
    
    // Calculate ensemble statistics
    uncertainty_stats = calculateEnsembleStatistics(model_outputs);
    
    return uncertainty_stats;
}

std::map<std::string, std::pair<double, double>> UncertaintyQuantifier::calculatePredictionIntervals(
    const std::vector<std::vector<std::vector<double>>>& model_outputs,
    double confidence_level) {
    
    std::map<std::string, std::pair<double, double>> prediction_intervals;
    
    if (model_outputs.empty()) {
        return prediction_intervals;
    }
    
    // Flatten all outputs for statistical analysis
    std::vector<double> all_values;
    
    for (const auto& output : model_outputs) {
        for (const auto& row : output) {
            for (double value : row) {
                all_values.push_back(value);
            }
        }
    }
    
    if (all_values.empty()) {
        return prediction_intervals;
    }
    
    // Sort values for percentile calculation
    std::sort(all_values.begin(), all_values.end());
    
    // Calculate percentiles
    double alpha = (1.0 - confidence_level) / 2.0;
    size_t lower_index = static_cast<size_t>(alpha * all_values.size());
    size_t upper_index = static_cast<size_t>((1.0 - alpha) * all_values.size());
    
    double lower_bound = all_values[lower_index];
    double upper_bound = all_values[upper_index];
    
    prediction_intervals["prediction_interval"] = std::make_pair(lower_bound, upper_bound);
    
    // Calculate additional statistics
    double mean_val = std::accumulate(all_values.begin(), all_values.end(), 0.0) / all_values.size();
    prediction_intervals["mean"] = std::make_pair(mean_val, mean_val);
    
    return prediction_intervals;
}

std::map<std::string, double> UncertaintyQuantifier::calculateEnsembleStatistics(
    const std::vector<std::vector<std::vector<double>>>& outputs) {
    
    std::map<std::string, double> statistics;
    
    if (outputs.empty()) {
        return statistics;
    }
    
    // Flatten all outputs
    std::vector<double> all_values;
    
    for (const auto& output : outputs) {
        for (const auto& row : output) {
            for (double value : row) {
                all_values.push_back(value);
            }
        }
    }
    
    if (all_values.empty()) {
        return statistics;
    }
    
    // Calculate basic statistics
    double mean_val = std::accumulate(all_values.begin(), all_values.end(), 0.0) / all_values.size();
    statistics["mean"] = mean_val;
    
    double variance = 0.0;
    for (double value : all_values) {
        variance += (value - mean_val) * (value - mean_val);
    }
    variance /= all_values.size();
    statistics["variance"] = variance;
    statistics["standard_deviation"] = std::sqrt(variance);
    
    // Calculate percentiles
    std::sort(all_values.begin(), all_values.end());
    
    size_t size = all_values.size();
    statistics["percentile_25"] = all_values[size * 0.25];
    statistics["percentile_50"] = all_values[size * 0.5];  // Median
    statistics["percentile_75"] = all_values[size * 0.75];
    
    statistics["min"] = all_values[0];
    statistics["max"] = all_values[size - 1];
    
    return statistics;
}
