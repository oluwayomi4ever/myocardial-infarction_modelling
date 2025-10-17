#include "DataProcessor.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>

// ECG Processor Implementation
ECGProcessor::ECGProcessor() : sampling_rate_(1000.0) {
    // Constructor
}

ECGProcessor::~ECGProcessor() {
    // Destructor
}

bool ECGProcessor::loadData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open ECG file " << filename << std::endl;
        return false;
    }
    
    try {
        std::string line;
        std::vector<double> temp_data;
        
        // Read header information if present
        std::getline(file, line);
        if (line.find("sampling_rate") != std::string::npos) {
            // Parse sampling rate from header
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                sampling_rate_ = std::stod(line.substr(pos + 1));
            }
        } else {
            // No header, reset file position
            file.seekg(0);
        }
        
        // Read ECG data
        double value;
        while (file >> value) {
            temp_data.push_back(value);
        }
        
        if (temp_data.empty()) {
            std::cerr << "Error: No ECG data found in file" << std::endl;
            return false;
        }
        
        // Reshape data into 2D grid (assuming 12 leads)
        int num_leads = 12;
        int num_samples = temp_data.size() / num_leads;
        
        ecg_data_.resize(num_leads, std::vector<double>(num_samples));
        time_stamps_.resize(num_samples);
        
        for (int i = 0; i < num_samples; ++i) {
            time_stamps_[i] = i / sampling_rate_;
            for (int lead = 0; lead < num_leads; ++lead) {
                ecg_data_[lead][i] = temp_data[i * num_leads + lead];
            }
        }
        
        file.close();
        std::cout << "ECG data loaded: " << num_leads << " leads, " << num_samples << " samples" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading ECG data: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool ECGProcessor::processData() {
    if (ecg_data_.empty()) {
        std::cerr << "Error: No ECG data to process" << std::endl;
        return false;
    }
    
    try {
        // Process each lead
        for (size_t lead = 0; lead < ecg_data_.size(); ++lead) {
            removeBaselineWander();
            applyBandpassFilter();
        }
        
        std::cout << "ECG data processing completed" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing ECG data: " << e.what() << std::endl;
        return false;
    }
}

bool ECGProcessor::saveProcessedData(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create output file " << filename << std::endl;
        return false;
    }
    
    try {
        // Write header
        file << "sampling_rate: " << sampling_rate_ << std::endl;
        file << "num_leads: " << ecg_data_.size() << std::endl;
        file << "num_samples: " << (ecg_data_.empty() ? 0 : ecg_data_[0].size()) << std::endl;
        
        // Write data
        for (size_t i = 0; i < (ecg_data_.empty() ? 0 : ecg_data_[0].size()); ++i) {
            for (size_t lead = 0; lead < ecg_data_.size(); ++lead) {
                file << ecg_data_[lead][i];
                if (lead < ecg_data_.size() - 1) file << " ";
            }
            file << std::endl;
        }
        
        file.close();
        std::cout << "Processed ECG data saved to " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving ECG data: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

const std::vector<std::vector<double>>& ECGProcessor::getProcessedData() const {
    return ecg_data_;
}

std::map<std::string, double> ECGProcessor::extractQRSParameters() {
    std::map<std::string, double> parameters;
    
    if (ecg_data_.empty()) {
        return parameters;
    }
    
    // Use lead II for QRS analysis (typically lead index 1)
    int lead_index = std::min(1, static_cast<int>(ecg_data_.size()) - 1);
    const auto& lead_data = ecg_data_[lead_index];
    
    // Detect R peaks
    auto r_peaks = detectRPeaks();
    
    if (r_peaks.size() < 2) {
        std::cerr << "Warning: Insufficient R peaks detected for QRS analysis" << std::endl;
        return parameters;
    }
    
    // Calculate QRS duration (simplified - using R-R intervals)
    std::vector<double> rr_intervals;
    for (size_t i = 1; i < r_peaks.size(); ++i) {
        double interval = (r_peaks[i] - r_peaks[i-1]) / sampling_rate_ * 1000.0; // Convert to ms
        rr_intervals.push_back(interval);
    }
    
    // Calculate statistics
    double mean_rr = std::accumulate(rr_intervals.begin(), rr_intervals.end(), 0.0) / rr_intervals.size();
    parameters["mean_rr_interval"] = mean_rr;
    parameters["heart_rate"] = 60000.0 / mean_rr; // BPM
    
    // Calculate QRS duration (simplified)
    parameters["qrs_duration"] = 80.0; // Typical value in ms
    
    return parameters;
}

std::string ECGProcessor::detectArrhythmias() {
    if (ecg_data_.empty()) {
        return "no_data";
    }
    
    auto parameters = extractQRSParameters();
    
    if (parameters.find("heart_rate") == parameters.end()) {
        return "insufficient_data";
    }
    
    double heart_rate = parameters["heart_rate"];
    
    // Simple arrhythmia detection
    if (heart_rate > 100) {
        return "tachycardia";
    } else if (heart_rate < 60) {
        return "bradycardia";
    } else {
        return "normal_rhythm";
    }
}

void ECGProcessor::applyBandpassFilter() {
    // Simplified bandpass filter (0.5-40 Hz)
    // This is a very basic implementation
    for (auto& lead_data : ecg_data_) {
        if (lead_data.size() < 3) continue;
        
        // Simple moving average filter (high-pass effect)
        std::vector<double> filtered(lead_data.size());
        int window_size = static_cast<int>(sampling_rate_ / 10.0); // 0.1 second window
        
        for (size_t i = window_size; i < lead_data.size() - window_size; ++i) {
            double sum = 0.0;
            for (int j = -window_size; j <= window_size; ++j) {
                sum += lead_data[i + j];
            }
            filtered[i] = lead_data[i] - sum / (2 * window_size + 1);
        }
        
        lead_data = filtered;
    }
}

void ECGProcessor::removeBaselineWander() {
    // Remove baseline wander using high-pass filtering
    for (auto& lead_data : ecg_data_) {
        if (lead_data.size() < 10) continue;
        
        // Simple baseline removal
        double baseline = std::accumulate(lead_data.begin(), lead_data.end(), 0.0) / lead_data.size();
        
        for (auto& sample : lead_data) {
            sample -= baseline;
        }
    }
}

std::vector<int> ECGProcessor::detectRPeaks() {
    std::vector<int> r_peaks;
    
    if (ecg_data_.empty()) {
        return r_peaks;
    }
    
    // Use lead II for R peak detection
    int lead_index = std::min(1, static_cast<int>(ecg_data_.size()) - 1);
    const auto& lead_data = ecg_data_[lead_index];
    
    if (lead_data.size() < 100) {
        return r_peaks;
    }
    
    // Simple R peak detection using threshold
    double threshold = *std::max_element(lead_data.begin(), lead_data.end()) * 0.6;
    
    for (size_t i = 1; i < lead_data.size() - 1; ++i) {
        if (lead_data[i] > threshold && 
            lead_data[i] > lead_data[i-1] && 
            lead_data[i] > lead_data[i+1]) {
            r_peaks.push_back(static_cast<int>(i));
        }
    }
    
    return r_peaks;
}

// MRI Processor Implementation
MRIProcessor::MRIProcessor(int width, int height) : width_(width), height_(height) {
    // Constructor
}

MRIProcessor::~MRIProcessor() {
    // Destructor
}

bool MRIProcessor::loadData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open MRI file " << filename << std::endl;
        return false;
    }
    
    try {
        // Read header information
        file >> width_ >> height_;
        
        // Resize and read image data
        mri_data_.resize(height_, std::vector<double>(width_, 0.0));
        
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                if (!(file >> mri_data_[y][x])) {
                    std::cerr << "Error reading MRI data at (" << x << ", " << y << ")" << std::endl;
                    return false;
                }
            }
        }
        
        file.close();
        std::cout << "MRI data loaded: " << width_ << "x" << height_ << " pixels" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading MRI data: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool MRIProcessor::processData() {
    if (mri_data_.empty()) {
        std::cerr << "Error: No MRI data to process" << std::endl;
        return false;
    }
    
    try {
        applyNoiseReduction();
        applyEdgeEnhancement();
        normalizeIntensity();
        
        std::cout << "MRI data processing completed" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing MRI data: " << e.what() << std::endl;
        return false;
    }
}

bool MRIProcessor::saveProcessedData(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create output file " << filename << std::endl;
        return false;
    }
    
    try {
        // Write header
        file << width_ << " " << height_ << std::endl;
        
        // Write image data
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                file << mri_data_[y][x];
                if (x < width_ - 1) file << " ";
            }
            file << std::endl;
        }
        
        file.close();
        std::cout << "Processed MRI data saved to " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving MRI data: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

const std::vector<std::vector<double>>& MRIProcessor::getProcessedData() const {
    return mri_data_;
}

std::vector<std::vector<int>> MRIProcessor::segmentTissue() {
    std::vector<std::vector<int>> tissue_map(height_, std::vector<int>(width_, 0));
    
    if (mri_data_.empty()) {
        return tissue_map;
    }
    
    // Simple threshold-based segmentation
    double min_val = *std::min_element(mri_data_[0].begin(), mri_data_[0].end());
    double max_val = *std::max_element(mri_data_[0].begin(), mri_data_[0].end());
    
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            double intensity = mri_data_[y][x];
            double normalized = (intensity - min_val) / (max_val - min_val);
            
            if (normalized < 0.3) {
                tissue_map[y][x] = 2; // Infarcted tissue
            } else if (normalized < 0.7) {
                tissue_map[y][x] = 1; // Ischemic tissue
            } else {
                tissue_map[y][x] = 0; // Normal tissue
            }
        }
    }
    
    return tissue_map;
}

std::vector<std::vector<double>> MRIProcessor::calculateWallThickness() {
    std::vector<std::vector<double>> thickness_map(height_, std::vector<double>(width_, 0.0));
    
    if (mri_data_.empty()) {
        return thickness_map;
    }
    
    // Simplified wall thickness calculation
    // This would typically involve more sophisticated image processing
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            // Placeholder calculation - would need actual anatomical analysis
            thickness_map[y][x] = 8.0 + 2.0 * sin(x * 0.1) * cos(y * 0.1);
        }
    }
    
    return thickness_map;
}

std::vector<std::vector<double>> MRIProcessor::extractPerfusionMap() {
    std::vector<std::vector<double>> perfusion_map(height_, std::vector<double>(width_, 0.0));
    
    if (mri_data_.empty()) {
        return perfusion_map;
    }
    
    // Simplified perfusion mapping based on intensity
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            perfusion_map[y][x] = mri_data_[y][x] / 1000.0; // Normalize
        }
    }
    
    return perfusion_map;
}

void MRIProcessor::applyNoiseReduction() {
    // Simple median filter for noise reduction
    std::vector<std::vector<double>> filtered = mri_data_;
    
    for (int y = 1; y < height_ - 1; ++y) {
        for (int x = 1; x < width_ - 1; ++x) {
            std::vector<double> neighbors;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    neighbors.push_back(mri_data_[y + dy][x + dx]);
                }
            }
            std::sort(neighbors.begin(), neighbors.end());
            filtered[y][x] = neighbors[4]; // Median of 9 neighbors
        }
    }
    
    mri_data_ = filtered;
}

void MRIProcessor::applyEdgeEnhancement() {
    // Simple edge enhancement using Laplacian
    std::vector<std::vector<double>> enhanced = mri_data_;
    
    for (int y = 1; y < height_ - 1; ++y) {
        for (int x = 1; x < width_ - 1; ++x) {
            double laplacian = mri_data_[y-1][x] + mri_data_[y+1][x] + 
                             mri_data_[y][x-1] + mri_data_[y][x+1] - 
                             4.0 * mri_data_[y][x];
            enhanced[y][x] = mri_data_[y][x] + 0.5 * laplacian;
        }
    }
    
    mri_data_ = enhanced;
}

void MRIProcessor::normalizeIntensity() {
    if (mri_data_.empty()) return;
    
    // Find min and max values
    double min_val = mri_data_[0][0];
    double max_val = mri_data_[0][0];
    
    for (const auto& row : mri_data_) {
        for (double val : row) {
            min_val = std::min(min_val, val);
            max_val = std::max(max_val, val);
        }
    }
    
    // Normalize to [0, 1]
    if (max_val > min_val) {
        for (auto& row : mri_data_) {
            for (double& val : row) {
                val = (val - min_val) / (max_val - min_val);
            }
        }
    }
}

// Echo Processor Implementation
EchoProcessor::EchoProcessor() {
    // Constructor
}

EchoProcessor::~EchoProcessor() {
    // Destructor
}

bool EchoProcessor::loadData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open Echo file " << filename << std::endl;
        return false;
    }
    
    try {
        // Read echo data (simplified format)
        std::string line;
        while (std::getline(file, line)) {
            std::vector<double> frame_data;
            std::istringstream iss(line);
            double value;
            while (iss >> value) {
                frame_data.push_back(value);
            }
            if (!frame_data.empty()) {
                echo_data_.push_back(frame_data);
            }
        }
        
        file.close();
        std::cout << "Echo data loaded: " << echo_data_.size() << " frames" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading Echo data: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool EchoProcessor::processData() {
    if (echo_data_.empty()) {
        std::cerr << "Error: No Echo data to process" << std::endl;
        return false;
    }
    
    try {
        trackBoundaries();
        std::cout << "Echo data processing completed" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing Echo data: " << e.what() << std::endl;
        return false;
    }
}

bool EchoProcessor::saveProcessedData(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create output file " << filename << std::endl;
        return false;
    }
    
    try {
        // Write processed echo data
        for (const auto& frame : echo_data_) {
            for (size_t i = 0; i < frame.size(); ++i) {
                file << frame[i];
                if (i < frame.size() - 1) file << " ";
            }
            file << std::endl;
        }
        
        file.close();
        std::cout << "Processed Echo data saved to " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving Echo data: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

const std::vector<std::vector<double>>& EchoProcessor::getProcessedData() const {
    return echo_data_;
}

double EchoProcessor::calculateEjectionFraction() {
    if (echo_data_.size() < 2) {
        return 0.0;
    }
    
    // Simplified ejection fraction calculation
    // Would need actual ventricular volume calculations
    double end_diastolic_volume = 120.0; // mL (typical value)
    double end_systolic_volume = 50.0;   // mL (typical value)
    
    double ejection_fraction = (end_diastolic_volume - end_systolic_volume) / end_diastolic_volume * 100.0;
    return ejection_fraction;
}

std::vector<std::vector<double>> EchoProcessor::analyzeWallMotion() {
    std::vector<std::vector<double>> wall_motion_scores;
    
    if (echo_data_.empty()) {
        return wall_motion_scores;
    }
    
    // Simplified wall motion analysis
    // Would need actual motion tracking
    int num_segments = 17; // Standard 17-segment model
    wall_motion_scores.resize(echo_data_.size(), std::vector<double>(num_segments, 0.0));
    
    for (size_t frame = 0; frame < echo_data_.size(); ++frame) {
        for (int segment = 0; segment < num_segments; ++segment) {
            // Placeholder wall motion score (0 = normal, 1 = hypokinetic, 2 = akinetic, 3 = dyskinetic)
            wall_motion_scores[frame][segment] = 0.0; // Normal motion
        }
    }
    
    return wall_motion_scores;
}

void EchoProcessor::trackBoundaries() {
    // Simplified boundary tracking
    // Would need sophisticated image processing algorithms
    for (auto& frame : echo_data_) {
        // Placeholder processing
        for (double& pixel : frame) {
            pixel *= 1.1; // Simple enhancement
        }
    }
}

std::vector<std::vector<double>> EchoProcessor::calculateStrain() {
    std::vector<std::vector<double>> strain_data;
    
    if (echo_data_.size() < 2) {
        return strain_data;
    }
    
    // Simplified strain calculation
    int num_segments = 17;
    strain_data.resize(echo_data_.size(), std::vector<double>(num_segments, 0.0));
    
    for (size_t frame = 0; frame < echo_data_.size(); ++frame) {
        for (int segment = 0; segment < num_segments; ++segment) {
            // Placeholder strain value (-20% to 20% typical range)
            strain_data[frame][segment] = -15.0; // Normal strain
        }
    }
    
    return strain_data;
}

// Data Integration Manager Implementation
DataIntegrationManager::DataIntegrationManager() {
    // Constructor
}

DataIntegrationManager::~DataIntegrationManager() {
    // Destructor
}

void DataIntegrationManager::addProcessor(const std::string& name, std::unique_ptr<DataProcessor> processor) {
    processors_[name] = std::move(processor);
}

bool DataIntegrationManager::loadAllData(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open configuration file " << config_file << std::endl;
        return false;
    }
    
    try {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string data_type, filename;
            if (iss >> data_type >> filename) {
                if (processors_.find(data_type) != processors_.end()) {
                    if (!processors_[data_type]->loadData(filename)) {
                        std::cerr << "Warning: Failed to load " << data_type << " data from " << filename << std::endl;
                    }
                } else {
                    std::cerr << "Warning: Unknown data type " << data_type << std::endl;
                }
            }
        }
        
        file.close();
        return validateDataConsistency();
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading configuration: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool DataIntegrationManager::processAllData() {
    bool all_successful = true;
    
    for (const auto& pair : processors_) {
        if (!pair.second->processData()) {
            std::cerr << "Warning: Failed to process " << pair.first << " data" << std::endl;
            all_successful = false;
        }
    }
    
    return all_successful;
}

std::map<std::string, double> DataIntegrationManager::generateModelParameters() {
    std::map<std::string, double> parameters;
    
    // Extract parameters from different data sources
    if (processors_.find("ecg") != processors_.end()) {
        auto ecg_processor = dynamic_cast<ECGProcessor*>(processors_["ecg"].get());
        if (ecg_processor) {
            auto ecg_params = ecg_processor->extractQRSParameters();
            parameters.insert(ecg_params.begin(), ecg_params.end());
        }
    }
    
    if (processors_.find("echo") != processors_.end()) {
        auto echo_processor = dynamic_cast<EchoProcessor*>(processors_["echo"].get());
        if (echo_processor) {
            parameters["ejection_fraction"] = echo_processor->calculateEjectionFraction();
        }
    }
    
    // Add default parameters
    parameters["heart_rate"] = 72.0; // BPM
    parameters["blood_pressure_systolic"] = 120.0; // mmHg
    parameters["blood_pressure_diastolic"] = 80.0; // mmHg
    
    return parameters;
}

std::map<std::string, std::vector<std::vector<double>>> DataIntegrationManager::createTissueMaps() {
    std::map<std::string, std::vector<std::vector<double>>> tissue_maps;
    
    if (processors_.find("mri") != processors_.end()) {
        auto mri_processor = dynamic_cast<MRIProcessor*>(processors_["mri"].get());
        if (mri_processor) {
            tissue_maps["wall_thickness"] = mri_processor->calculateWallThickness();
            tissue_maps["perfusion"] = mri_processor->extractPerfusionMap();
        }
    }
    
    return tissue_maps;
}

std::map<std::string, std::vector<std::vector<int>>> DataIntegrationManager::createTissueSegmentation() {
    std::map<std::string, std::vector<std::vector<int>>> segmentation_maps;
    
    if (processors_.find("mri") != processors_.end()) {
        auto mri_processor = dynamic_cast<MRIProcessor*>(processors_["mri"].get());
        if (mri_processor) {
            segmentation_maps["tissue_type"] = mri_processor->segmentTissue();
        }
    }
    
    return segmentation_maps;
}

bool DataIntegrationManager::validateDataConsistency() {
    // Check if all required data sources are present
    std::vector<std::string> required_sources = {"ecg", "mri"};
    
    for (const auto& source : required_sources) {
        if (processors_.find(source) == processors_.end()) {
            std::cerr << "Warning: Missing required data source: " << source << std::endl;
            return false;
        }
    }
    
    return true;
}
