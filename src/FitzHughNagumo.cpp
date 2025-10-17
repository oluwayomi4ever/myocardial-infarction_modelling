#include "FitzHughNagumo.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <random>

FitzHughNagumo::FitzHughNagumo(int width, int height, double dt)
    : width_(width), height_(height), dt_(dt), time_(0.0),
      a_(0.1), b_(0.5), c_(1.0), d_(0.0), du_(0.1), dv_(0.0) {
    
    // Initialize state grids
    u_.resize(height_);
    v_.resize(height_);
    u_new_.resize(height_);
    v_new_.resize(height_);
    stimulus_.resize(height_);
    
    for (int i = 0; i < height_; ++i) {
        u_[i].resize(width_, 0.0);
        v_[i].resize(width_, 0.0);
        u_new_[i].resize(width_, 0.0);
        v_new_[i].resize(width_, 0.0);
        stimulus_[i].resize(width_, 0.0);
    }
}

FitzHughNagumo::~FitzHughNagumo() {
    // Destructor - vectors will automatically clean up
}

void FitzHughNagumo::initialize() {
    // Initialize with small random perturbations
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-0.01, 0.01);
    
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            u_[y][x] = dis(gen);
            v_[y][x] = dis(gen);
            stimulus_[y][x] = 0.0;
        }
    }
    
    time_ = 0.0;
}

void FitzHughNagumo::setParameters(double a, double b, double c, double d) {
    a_ = a;
    b_ = b;
    c_ = c;
    d_ = d;
}

void FitzHughNagumo::setDiffusionCoefficients(double du, double dv) {
    du_ = du;
    dv_ = dv;
}

void FitzHughNagumo::setInitialConditions(const std::vector<std::vector<double>>& u_init,
                                         const std::vector<std::vector<double>>& v_init) {
    if (u_init.size() != height_ || v_init.size() != height_ ||
        (height_ > 0 && (u_init[0].size() != width_ || v_init[0].size() != width_))) {
        std::cerr << "Error: Initial condition dimensions do not match grid size" << std::endl;
        return;
    }
    
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            u_[y][x] = u_init[y][x];
            v_[y][x] = v_init[y][x];
        }
    }
}

void FitzHughNagumo::addStimulus(int x, int y, double strength, double duration) {
    if (!isValidCoordinate(x, y)) {
        std::cerr << "Error: Invalid stimulus coordinates (" << x << ", " << y << ")" << std::endl;
        return;
    }
    
    stimulus_[y][x] = strength;
    // Note: In a more sophisticated implementation, duration would be handled
    // by tracking stimulus timing and decay
}

void FitzHughNagumo::step() {
    // Apply diffusion and reaction terms
    std::vector<std::vector<double>> du_dt(height_, std::vector<double>(width_, 0.0));
    std::vector<std::vector<double>> dv_dt(height_, std::vector<double>(width_, 0.0));
    
    // Calculate diffusion terms
    if (du_ > 0.0) {
        applyDiffusion(u_, du_, du_dt);
    }
    if (dv_ > 0.0) {
        applyDiffusion(v_, dv_, dv_dt);
    }
    
    // Add reaction terms and update
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            double u_val = u_[y][x];
            double v_val = v_[y][x];
            double stim_val = stimulus_[y][x];
            
            du_dt[y][x] += reactionU(u_val, v_val, stim_val);
            dv_dt[y][x] += reactionV(u_val, v_val);
            
            // Update state using Euler method
            u_new_[y][x] = u_val + dt_ * du_dt[y][x];
            v_new_[y][x] = v_val + dt_ * dv_dt[y][x];
        }
    }
    
    // Swap grids
    u_.swap(u_new_);
    v_.swap(v_new_);
    
    time_ += dt_;
}

void FitzHughNagumo::run(int steps) {
    for (int i = 0; i < steps; ++i) {
        step();
    }
}

const std::vector<std::vector<double>>& FitzHughNagumo::getU() const {
    return u_;
}

const std::vector<std::vector<double>>& FitzHughNagumo::getV() const {
    return v_;
}

double FitzHughNagumo::getTime() const {
    return time_;
}

bool FitzHughNagumo::saveState(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create file " << filename << std::endl;
        return false;
    }
    
    try {
        // Write header
        file << width_ << " " << height_ << " " << time_ << std::endl;
        file << a_ << " " << b_ << " " << c_ << " " << d_ << std::endl;
        file << du_ << " " << dv_ << std::endl;
        
        // Write u values
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                file << u_[y][x];
                if (x < width_ - 1) file << " ";
            }
            file << std::endl;
        }
        
        // Write v values
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                file << v_[y][x];
                if (x < width_ - 1) file << " ";
            }
            file << std::endl;
        }
        
        file.close();
        std::cout << "State saved to " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving state: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool FitzHughNagumo::loadState(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    try {
        // Read header
        int file_width, file_height;
        file >> file_width >> file_height >> time_;
        
        if (file_width != width_ || file_height != height_) {
            std::cerr << "Error: File grid size does not match current grid size" << std::endl;
            return false;
        }
        
        file >> a_ >> b_ >> c_ >> d_;
        file >> du_ >> dv_;
        
        // Read u values
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                if (!(file >> u_[y][x])) {
                    std::cerr << "Error reading u values" << std::endl;
                    return false;
                }
            }
        }
        
        // Read v values
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                if (!(file >> v_[y][x])) {
                    std::cerr << "Error reading v values" << std::endl;
                    return false;
                }
            }
        }
        
        file.close();
        std::cout << "State loaded from " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading state: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

void FitzHughNagumo::applyDiffusion(const std::vector<std::vector<double>>& grid,
                                   double coeff,
                                   std::vector<std::vector<double>>& result) {
    // Apply 5-point stencil for 2D diffusion
    for (int y = 1; y < height_ - 1; ++y) {
        for (int x = 1; x < width_ - 1; ++x) {
            double laplacian = grid[y-1][x] + grid[y+1][x] + grid[y][x-1] + grid[y][x+1] - 4.0 * grid[y][x];
            result[y][x] = coeff * laplacian;
        }
    }
}

double FitzHughNagumo::reactionU(double u_val, double v_val, double stim_val) {
    // FitzHugh-Nagumo reaction term for u: du/dt = u - u^3/3 - v + stimulus
    return u_val - u_val * u_val * u_val / 3.0 - v_val + stim_val;
}

double FitzHughNagumo::reactionV(double u_val, double v_val) {
    // FitzHugh-Nagumo reaction term for v: dv/dt = (u + a - b*v)/c
    return (u_val + a_ - b_ * v_val) / c_;
}

bool FitzHughNagumo::isValidCoordinate(int x, int y) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}
