#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

/**
 * @file DataProcessor.h
 * @brief Data preprocessing and integration for clinical MI data
 */

#include <vector>
#include <string>
#include <map>
#include <memory>

/**
 * @brief Base class for clinical data processors
 */
class DataProcessor {
public:
    virtual ~DataProcessor() = default;
    
    /**
     * @brief Load data from file
     * @param filename Input filename
     * @return true if successful
     */
    virtual bool loadData(const std::string& filename) = 0;
    
    /**
     * @brief Process and preprocess the data
     * @return true if successful
     */
    virtual bool processData() = 0;
    
    /**
     * @brief Save processed data
     * @param filename Output filename
     * @return true if successful
     */
    virtual bool saveProcessedData(const std::string& filename) const = 0;
    
    /**
     * @brief Get processed data as 2D grid
     * @return Processed data grid
     */
    virtual const std::vector<std::vector<double>>& getProcessedData() const = 0;
};

/**
 * @brief ECG data processor
 */
class ECGProcessor : public DataProcessor {
public:
    ECGProcessor();
    ~ECGProcessor() override;
    
    bool loadData(const std::string& filename) override;
    bool processData() override;
    bool saveProcessedData(const std::string& filename) const override;
    const std::vector<std::vector<double>>& getProcessedData() const override;
    
    /**
     * @brief Extract QRS complex parameters
     * @return Map of QRS parameters
     */
    std::map<std::string, double> extractQRSParameters();
    
    /**
     * @brief Detect arrhythmias
     * @return Arrhythmia type string
     */
    std::string detectArrhythmias();

private:
    std::vector<std::vector<double>> ecg_data_;
    std::vector<double> time_stamps_;
    double sampling_rate_;
    
    /**
     * @brief Apply bandpass filter
     */
    void applyBandpassFilter();
    
    /**
     * @brief Remove baseline wander
     */
    void removeBaselineWander();
    
    /**
     * @brief Detect R peaks
     */
    std::vector<int> detectRPeaks();
};

/**
 * @brief MRI data processor for cardiac imaging
 */
class MRIProcessor : public DataProcessor {
public:
    MRIProcessor(int width, int height);
    ~MRIProcessor() override;
    
    bool loadData(const std::string& filename) override;
    bool processData() override;
    bool saveProcessedData(const std::string& filename) const override;
    const std::vector<std::vector<double>>& getProcessedData() const override;
    
    /**
     * @brief Segment myocardial tissue
     * @return Tissue type grid (0=normal, 1=ischemic, 2=infarcted)
     */
    std::vector<std::vector<int>> segmentTissue();
    
    /**
     * @brief Calculate wall thickness
     * @return Wall thickness grid
     */
    std::vector<std::vector<double>> calculateWallThickness();
    
    /**
     * @brief Extract perfusion parameters
     * @return Perfusion map
     */
    std::vector<std::vector<double>> extractPerfusionMap();

private:
    int width_, height_;
    std::vector<std::vector<double>> mri_data_;
    
    /**
     * @brief Apply noise reduction
     */
    void applyNoiseReduction();
    
    /**
     * @brief Apply edge enhancement
     */
    void applyEdgeEnhancement();
    
    /**
     * @brief Apply intensity normalization
     */
    void normalizeIntensity();
};

/**
 * @brief Echocardiogram data processor
 */
class EchoProcessor : public DataProcessor {
public:
    EchoProcessor();
    ~EchoProcessor() override;
    
    bool loadData(const std::string& filename) override;
    bool processData() override;
    bool saveProcessedData(const std::string& filename) const override;
    const std::vector<std::vector<double>>& getProcessedData() const override;
    
    /**
     * @brief Calculate ejection fraction
     * @return Ejection fraction percentage
     */
    double calculateEjectionFraction();
    
    /**
     * @brief Analyze wall motion
     * @return Wall motion score grid
     */
    std::vector<std::vector<double>> analyzeWallMotion();

private:
    std::vector<std::vector<double>> echo_data_;
    
    /**
     * @brief Track cardiac boundaries
     */
    void trackBoundaries();
    
    /**
     * @brief Calculate strain parameters
     */
    std::vector<std::vector<double>> calculateStrain();
};

/**
 * @brief Data integration manager for combining multiple data sources
 */
class DataIntegrationManager {
public:
    DataIntegrationManager();
    ~DataIntegrationManager();
    
    /**
     * @brief Add data processor
     * @param name Data source name
     * @param processor Data processor instance
     */
    void addProcessor(const std::string& name, std::unique_ptr<DataProcessor> processor);
    
    /**
     * @brief Load all data sources
     * @param config_file Configuration file with data paths
     * @return true if all successful
     */
    bool loadAllData(const std::string& config_file);
    
    /**
     * @brief Process all data sources
     * @return true if all successful
     */
    bool processAllData();
    
    /**
     * @brief Generate integrated model parameters
     * @return Map of parameter names to values
     */
    std::map<std::string, double> generateModelParameters();
    
    /**
     * @brief Create tissue property maps
     * @return Map of property names to 2D grids
     */
    std::map<std::string, std::vector<std::vector<double>>> createTissueMaps();
    
    /**
     * @brief Create tissue segmentation map
     * @return Map of tissue types (0=normal, 1=ischemic, 2=infarcted)
     */
    std::map<std::string, std::vector<std::vector<int>>> createTissueSegmentation();

private:
    std::map<std::string, std::unique_ptr<DataProcessor>> processors_;
    
    /**
     * @brief Validate data consistency
     * @return true if data is consistent
     */
    bool validateDataConsistency();
};

#endif // DATAPROCESSOR_H

