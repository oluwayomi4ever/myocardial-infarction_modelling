#include "DTM.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>

DTM::DTM(int width, int height, double cell_size)
    : width_(width), height_(height), cell_size_(cell_size) {
    // Initialize elevation data grid
    elevation_data_.resize(height_);
    for (int i = 0; i < height_; ++i) {
        elevation_data_[i].resize(width_, 0.0);
    }
}

DTM::~DTM() {
    // Destructor - vectors will automatically clean up
}

bool DTM::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    try {
        // Read header information
        file >> width_ >> height_ >> cell_size_;
        
        // Resize elevation data
        elevation_data_.resize(height_);
        for (int i = 0; i < height_; ++i) {
            elevation_data_[i].resize(width_);
        }
        
        // Read elevation data
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                if (!(file >> elevation_data_[y][x])) {
                    std::cerr << "Error: Failed to read elevation data at (" << x << ", " << y << ")" << std::endl;
                    return false;
                }
            }
        }
        
        file.close();
        std::cout << "DTM loaded successfully: " << width_ << "x" << height_ << " grid, cell size: " << cell_size_ << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading DTM file: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool DTM::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create file " << filename << std::endl;
        return false;
    }
    
    try {
        // Write header information
        file << width_ << " " << height_ << " " << cell_size_ << std::endl;
        
        // Write elevation data
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                file << elevation_data_[y][x];
                if (x < width_ - 1) {
                    file << " ";
                }
            }
            file << std::endl;
        }
        
        file.close();
        std::cout << "DTM saved successfully to " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving DTM file: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

double DTM::getElevation(int x, int y) const {
    if (!isValidCoordinate(x, y)) {
        std::cerr << "Error: Invalid coordinates (" << x << ", " << y << ")" << std::endl;
        return 0.0;
    }
    return elevation_data_[y][x];
}

void DTM::setElevation(int x, int y, double elevation) {
    if (!isValidCoordinate(x, y)) {
        std::cerr << "Error: Invalid coordinates (" << x << ", " << y << ")" << std::endl;
        return;
    }
    elevation_data_[y][x] = elevation;
}

std::pair<int, int> DTM::getDimensions() const {
    return std::make_pair(width_, height_);
}

double DTM::getCellSize() const {
    return cell_size_;
}

double DTM::calculateSlope(int x, int y) const {
    if (!isValidCoordinate(x, y) || x == 0 || x == width_ - 1 || y == 0 || y == height_ - 1) {
        return 0.0;  // Cannot calculate slope at boundaries
    }
    
    // Calculate slope using the 3x3 neighborhood
    double dz_dx = (getElevation(x + 1, y) - getElevation(x - 1, y)) / (2.0 * cell_size_);
    double dz_dy = (getElevation(x, y + 1) - getElevation(x, y - 1)) / (2.0 * cell_size_);
    
    double slope_radians = std::atan(std::sqrt(dz_dx * dz_dx + dz_dy * dz_dy));
    return slope_radians * 180.0 / M_PI;  // Convert to degrees
}

double DTM::calculateAspect(int x, int y) const {
    if (!isValidCoordinate(x, y) || x == 0 || x == width_ - 1 || y == 0 || y == height_ - 1) {
        return 0.0;  // Cannot calculate aspect at boundaries
    }
    
    // Calculate aspect using the 3x3 neighborhood
    double dz_dx = (getElevation(x + 1, y) - getElevation(x - 1, y)) / (2.0 * cell_size_);
    double dz_dy = (getElevation(x, y + 1) - getElevation(x, y - 1)) / (2.0 * cell_size_);
    
    double aspect_radians = std::atan2(dz_dy, dz_dx);
    double aspect_degrees = aspect_radians * 180.0 / M_PI;
    
    // Convert to compass bearing (0-360 degrees, where 0 is North)
    aspect_degrees = 90.0 - aspect_degrees;
    if (aspect_degrees < 0) {
        aspect_degrees += 360.0;
    }
    
    return aspect_degrees;
}

bool DTM::isValidCoordinate(int x, int y) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}
