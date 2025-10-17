#ifndef DTM_H
#define DTM_H

/**
 * @file DTM.h
 * @brief Differential Transform Method (DTM) class for solving differential equations in MI modeling
 */

#include <vector>
#include <string>

class DTM {
public:
    /**
     * @brief Constructor for Differential Transform Method solver
     * @param grid_width Width of the computational grid
     * @param grid_height Height of the computational grid
     * @param spatial_step Spatial discretization step size
     */
    DTM(int grid_width, int grid_height, double spatial_step = 1.0);
    
    /**
     * @brief Destructor
     */
    ~DTM();
    
    /**
     * @brief Load DTM data from file
     * @param filename Path to the DTM data file
     * @return true if successful, false otherwise
     */
    bool loadFromFile(const std::string& filename);
    
    /**
     * @brief Save DTM data to file
     * @param filename Path to save the DTM data
     * @return true if successful, false otherwise
     */
    bool saveToFile(const std::string& filename) const;
    
    /**
     * @brief Get elevation at specific coordinates
     * @param x X coordinate
     * @param y Y coordinate
     * @return Elevation value
     */
    double getElevation(int x, int y) const;
    
    /**
     * @brief Set elevation at specific coordinates
     * @param x X coordinate
     * @param y Y coordinate
     * @param elevation Elevation value to set
     */
    void setElevation(int x, int y, double elevation);
    
    /**
     * @brief Get grid dimensions
     * @return Pair of (width, height)
     */
    std::pair<int, int> getDimensions() const;
    
    /**
     * @brief Get cell size
     * @return Cell size value
     */
    double getCellSize() const;
    
    /**
     * @brief Calculate terrain slope at given point
     * @param x X coordinate
     * @param y Y coordinate
     * @return Slope in degrees
     */
    double calculateSlope(int x, int y) const;
    
    /**
     * @brief Calculate terrain aspect at given point
     * @param x X coordinate
     * @param y Y coordinate
     * @return Aspect in degrees
     */
    double calculateAspect(int x, int y) const;

private:
    int width_;           ///< Width of the terrain grid
    int height_;          ///< Height of the terrain grid
    double cell_size_;    ///< Size of each grid cell
    std::vector<std::vector<double>> elevation_data_; ///< 2D grid of elevation data
    
    /**
     * @brief Check if coordinates are within bounds
     * @param x X coordinate
     * @param y Y coordinate
     * @return true if coordinates are valid
     */
    bool isValidCoordinate(int x, int y) const;
};

#endif // DTM_H
