#include "CardiacElectrophysiology.h"
#include <iostream>
#include <cmath>
#include <random>
#include <map>

// Base class implementation
CardiacElectrophysiology::CardiacElectrophysiology(int width, int height, double dt)
    : width_(width), height_(height), dt_(dt), time_(0.0), conductivity_(1.0) {
    
    mi_region_.resize(height_);
    for (int i = 0; i < height_; ++i) {
        mi_region_[i].resize(width_, false);
    }
}

void CardiacElectrophysiology::run(int steps) {
    for (int i = 0; i < steps; ++i) {
        step();
    }
}

void CardiacElectrophysiology::setMIRegion(const std::vector<std::vector<bool>>& mi_region) {
    if (mi_region.size() != height_ || (height_ > 0 && mi_region[0].size() != width_)) {
        std::cerr << "Error: MI region dimensions do not match grid size" << std::endl;
        return;
    }
    mi_region_ = mi_region;
}

void CardiacElectrophysiology::applyDiffusion(const std::vector<std::vector<double>>& grid,
                                            std::vector<std::vector<double>>& result) {
    // Apply 5-point stencil for 2D diffusion
    for (int y = 1; y < height_ - 1; ++y) {
        for (int x = 1; x < width_ - 1; ++x) {
            // Skip diffusion in MI regions (scar tissue)
            if (mi_region_[y][x]) {
                result[y][x] = 0.0;
                continue;
            }
            
            double laplacian = grid[y-1][x] + grid[y+1][x] + grid[y][x-1] + grid[y][x+1] - 4.0 * grid[y][x];
            result[y][x] = conductivity_ * laplacian;
        }
    }
}

// Luo-Rudy Model Implementation
LuoRudyModel::LuoRudyModel(int width, int height, double dt)
    : CardiacElectrophysiology(width, height, dt) {
    
    // Initialize state variables
    V_.resize(height_);
    m_.resize(height_);
    h_.resize(height_);
    j_.resize(height_);
    xr_.resize(height_);
    xs_.resize(height_);
    d_.resize(height_);
    f_.resize(height_);
    fca_.resize(height_);
    Cai_.resize(height_);
    CaSR_.resize(height_);
    
    for (int i = 0; i < height_; ++i) {
        V_[i].resize(width_, -84.0);  // Resting potential
        m_[i].resize(width_, 0.0);
        h_[i].resize(width_, 1.0);
        j_[i].resize(width_, 1.0);
        xr_[i].resize(width_, 0.0);
        xs_[i].resize(width_, 0.0);
        d_[i].resize(width_, 0.0);
        f_[i].resize(width_, 1.0);
        fca_[i].resize(width_, 1.0);
        Cai_[i].resize(width_, 0.0002);
        CaSR_[i].resize(width_, 0.2);
    }
    
    // Set default parameters for normal tissue
    setCellType("normal");
}

LuoRudyModel::~LuoRudyModel() {
    // Destructor - vectors will automatically clean up
}

void LuoRudyModel::step() {
    std::vector<std::vector<double>> V_new(height_, std::vector<double>(width_, 0.0));
    std::vector<std::vector<double>> dV_dt(height_, std::vector<double>(width_, 0.0));
    
    // Apply diffusion
    applyDiffusion(V_, dV_dt);
    
    // Add reaction terms and update state variables
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (mi_region_[y][x]) {
                // Scar tissue - no electrical activity
                V_new[y][x] = V_[y][x];
                continue;
            }
            
            auto currents = calculateIonicCurrents(x, y);
            
            // Total ionic current
            double I_total = currents["INa"] + currents["ICaL"] + currents["IK"] + 
                           currents["IK1"] + currents["Ib"] + currents["ICaT"];
            
            // Update membrane potential: dV/dt = -(I_ion + I_diff)/C_m
            double dV = -(I_total + dV_dt[y][x]) * dt_;
            V_new[y][x] = V_[y][x] + dV;
            
            // Update gating variables (simplified)
            double V_val = V_[y][x];
            
            // Sodium gates
            double alpha_m = 0.32 * (V_val + 47.13) / (1 - exp(-0.1 * (V_val + 47.13)));
            double beta_m = 0.08 * exp(-V_val / 11.0);
            m_[y][x] += dt_ * (alpha_m * (1 - m_[y][x]) - beta_m * m_[y][x]);
            
            double alpha_h = 0.135 * exp(-(V_val + 80) / 6.8);
            double beta_h = 3.56 * exp(0.079 * V_val) + 3.1e6 * exp(0.35 * V_val);
            h_[y][x] += dt_ * (alpha_h * (1 - h_[y][x]) - beta_h * h_[y][x]);
            
            // Potassium gates (simplified)
            double alpha_xr = 0.0005 * exp(0.083 * (V_val + 50)) / (1 + exp(0.057 * (V_val + 50)));
            double beta_xr = 0.0013 * exp(-0.06 * (V_val + 20)) / (1 + exp(-0.04 * (V_val + 20)));
            xr_[y][x] += dt_ * (alpha_xr * (1 - xr_[y][x]) - beta_xr * xr_[y][x]);
            
            // Calcium handling (simplified)
            Cai_[y][x] += dt_ * 0.001 * (-currents["ICaL"] - 0.0001 * Cai_[y][x]);
            Cai_[y][x] = std::max(0.0001, std::min(0.01, Cai_[y][x]));
        }
    }
    
    V_ = V_new;
    time_ += dt_;
}

void LuoRudyModel::setCellType(const std::string& cell_type) {
    if (cell_type == "normal") {
        GNa_ = 23.0;   // mS/μF
        Gsi_ = 0.09;   // mS/μF
        GK_ = 0.282;   // mS/μF
        GK1_ = 0.6047; // mS/μF
        Gb_ = 0.03921; // mS/μF
        GCaL_ = 0.000175; // mS/μF
    } else if (cell_type == "ischemic") {
        // Reduced conductances in ischemic tissue
        GNa_ = 15.0;
        Gsi_ = 0.06;
        GK_ = 0.2;
        GK1_ = 0.4;
        Gb_ = 0.03;
        GCaL_ = 0.00012;
    } else if (cell_type == "infarcted") {
        // Severely reduced conductances in infarcted tissue
        GNa_ = 2.0;
        Gsi_ = 0.01;
        GK_ = 0.05;
        GK1_ = 0.1;
        Gb_ = 0.01;
        GCaL_ = 0.00002;
    }
}

std::map<std::string, double> LuoRudyModel::calculateIonicCurrents(int x, int y) {
    double V = V_[y][x];
    double Cai = Cai_[y][x];
    
    std::map<std::string, double> currents;
    
    // Fast sodium current: INa = GNa * m^3 * h * j * (V - ENa)
    double ENa = 54.4; // mV
    currents["INa"] = GNa_ * m_[y][x] * m_[y][x] * m_[y][x] * h_[y][x] * j_[y][x] * (V - ENa);
    
    // L-type calcium current: ICaL = GCaL * d * f * fca * (V - ECa)
    double ECa = 130.0; // mV
    currents["ICaL"] = GCaL_ * d_[y][x] * f_[y][x] * fca_[y][x] * (V - ECa);
    
    // Delayed rectifier potassium: IK = GK * xr * xs * (V - EK)
    double EK = -77.0; // mV
    currents["IK"] = GK_ * xr_[y][x] * xs_[y][x] * (V - EK);
    
    // Inward rectifier potassium: IK1 = GK1 * (V - EK) / (1 + exp(0.07 * (V + 80)))
    currents["IK1"] = GK1_ * (V - EK) / (1 + exp(0.07 * (V + 80)));
    
    // Background current: Ib = Gb * (V + 59.87)
    currents["Ib"] = Gb_ * (V + 59.87);
    
    // T-type calcium current (simplified)
    currents["ICaT"] = 0.0005 * d_[y][x] * (V - ECa);
    
    return currents;
}

std::map<std::string, std::vector<std::vector<double>>> LuoRudyModel::getIonicCurrents() const {
    std::map<std::string, std::vector<std::vector<double>>> current_maps;
    
    // Initialize current maps
    std::vector<std::string> current_names = {"INa", "ICaL", "IK", "IK1", "Ib", "ICaT"};
    for (const auto& name : current_names) {
        current_maps[name].resize(height_, std::vector<double>(width_, 0.0));
    }
    
    // Calculate currents for all grid points
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            auto currents = const_cast<LuoRudyModel*>(this)->calculateIonicCurrents(x, y);
            for (const auto& pair : currents) {
                current_maps[pair.first][y][x] = pair.second;
            }
        }
    }
    
    return current_maps;
}

// Ten Tusscher Model Implementation
TenTusscherModel::TenTusscherModel(int width, int height, double dt)
    : CardiacElectrophysiology(width, height, dt) {
    
    // Initialize state variables
    V_.resize(height_);
    m_.resize(height_);
    h_.resize(height_);
    j_.resize(height_);
    oa_.resize(height_);
    oi_.resize(height_);
    d_.resize(height_);
    f_.resize(height_);
    fca_.resize(height_);
    u_.resize(height_);
    v_.resize(height_);
    w_.resize(height_);
    Cai_.resize(height_);
    CaSR_.resize(height_);
    CaSS_.resize(height_);
    Nai_.resize(height_);
    Ki_.resize(height_);
    
    for (int i = 0; i < height_; ++i) {
        V_[i].resize(width_, -86.2);  // Resting potential
        m_[i].resize(width_, 0.0);
        h_[i].resize(width_, 0.75);
        j_[i].resize(width_, 0.75);
        oa_[i].resize(width_, 0.0);
        oi_[i].resize(width_, 1.0);
        d_[i].resize(width_, 0.0);
        f_[i].resize(width_, 1.0);
        fca_[i].resize(width_, 1.0);
        u_[i].resize(width_, 0.0);
        v_[i].resize(width_, 1.0);
        w_[i].resize(width_, 1.0);
        Cai_[i].resize(width_, 0.0002);
        CaSR_[i].resize(width_, 0.2);
        CaSS_[i].resize(width_, 0.0002);
        Nai_[i].resize(width_, 11.6);
        Ki_[i].resize(width_, 138.3);
    }
    
    // Set default variant
    setVariant("epi");
}

TenTusscherModel::~TenTusscherModel() {
    // Destructor - vectors will automatically clean up
}

void TenTusscherModel::step() {
    std::vector<std::vector<double>> V_new(height_, std::vector<double>(width_, 0.0));
    std::vector<std::vector<double>> dV_dt(height_, std::vector<double>(width_, 0.0));
    
    // Apply diffusion
    applyDiffusion(V_, dV_dt);
    
    // Add reaction terms and update state variables
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (mi_region_[y][x]) {
                // Scar tissue - no electrical activity
                V_new[y][x] = V_[y][x];
                continue;
            }
            
            auto currents = calculateIonicCurrents(x, y);
            
            // Total ionic current
            double I_total = currents["INa"] + currents["ICaL"] + currents["IKr"] + 
                           currents["IKs"] + currents["IK1"] + currents["Ito"] + 
                           currents["INaCa"] + currents["INaK"];
            
            // Update membrane potential
            double dV = -(I_total + dV_dt[y][x]) * dt_;
            V_new[y][x] = V_[y][x] + dV;
            
            // Update gating variables (simplified)
            double V_val = V_[y][x];
            
            // Sodium gates (simplified)
            double alpha_m = 0.32 * (V_val + 47.13) / (1 - exp(-0.1 * (V_val + 47.13)));
            double beta_m = 0.08 * exp(-V_val / 11.0);
            m_[y][x] += dt_ * (alpha_m * (1 - m_[y][x]) - beta_m * m_[y][x]);
            
            // Potassium gates (simplified)
            double alpha_xr = 0.0005 * exp(0.083 * (V_val + 50)) / (1 + exp(0.057 * (V_val + 50)));
            double beta_xr = 0.0013 * exp(-0.06 * (V_val + 20)) / (1 + exp(-0.04 * (V_val + 20)));
            u_[y][x] += dt_ * (alpha_xr * (1 - u_[y][x]) - beta_xr * u_[y][x]);
            
            // Calcium handling (simplified)
            Cai_[y][x] += dt_ * 0.001 * (-currents["ICaL"] - 0.0001 * Cai_[y][x]);
            Cai_[y][x] = std::max(0.0001, std::min(0.01, Cai_[y][x]));
        }
    }
    
    V_ = V_new;
    time_ += dt_;
}

void TenTusscherModel::setVariant(const std::string& variant) {
    if (variant == "epi") {
        // Epicardial cell parameters
        GNa_ = 75.0;
        GCaL_ = 0.000175;
        GKr_ = 0.046;
        GKs_ = 0.0034;
        GK1_ = 0.1908;
        Gto_ = 0.294;
        GNaCa_ = 1000.0;
        GNaK_ = 1.362;
    } else if (variant == "endo") {
        // Endocardial cell parameters
        GNa_ = 75.0;
        GCaL_ = 0.000175;
        GKr_ = 0.023;
        GKs_ = 0.0034;
        GK1_ = 0.1908;
        Gto_ = 0.073;
        GNaCa_ = 1000.0;
        GNaK_ = 1.362;
    } else if (variant == "mid") {
        // Mid-myocardial cell parameters
        GNa_ = 75.0;
        GCaL_ = 0.000175;
        GKr_ = 0.023;
        GKs_ = 0.0034;
        GK1_ = 0.1908;
        Gto_ = 0.294;
        GNaCa_ = 1000.0;
        GNaK_ = 1.362;
    }
}

std::map<std::string, double> TenTusscherModel::calculateIonicCurrents(int x, int y) {
    double V = V_[y][x];
    double Cai = Cai_[y][x];
    double Nai = Nai_[y][x];
    double Ki = Ki_[y][x];
    
    std::map<std::string, double> currents;
    
    // Fast sodium current
    double ENa = 54.4;
    currents["INa"] = GNa_ * m_[y][x] * m_[y][x] * m_[y][x] * h_[y][x] * j_[y][x] * (V - ENa);
    
    // L-type calcium current
    double ECa = 130.0;
    currents["ICaL"] = GCaL_ * d_[y][x] * f_[y][x] * fca_[y][x] * (V - ECa);
    
    // Rapid delayed rectifier potassium
    double EKr = -77.0;
    currents["IKr"] = GKr_ * std::sqrt(Ki / 5.4) * u_[y][x] * (V - EKr);
    
    // Slow delayed rectifier potassium
    double EKs = -77.0;
    currents["IKs"] = GKs_ * v_[y][x] * (V - EKs);
    
    // Inward rectifier potassium
    currents["IK1"] = GK1_ * std::sqrt(Ki / 5.4) * (V - EKr) / (1 + exp(0.07 * (V + 80)));
    
    // Transient outward potassium
    currents["Ito"] = Gto_ * oa_[y][x] * oi_[y][x] * (V - EKr);
    
    // Sodium-calcium exchanger
    currents["INaCa"] = GNaCa_ * (exp(0.03743 * V) * Nai * Nai * Nai * Cai - 
                                 exp(-0.03743 * V) * 1.0 * 1.0 * 1.0 * 1.4) / 
                        (1 + 0.1 * exp(-0.03743 * V));
    
    // Sodium-potassium pump
    currents["INaK"] = GNaK_ * Ki / (Ki + 1.0) * Nai / (Nai + 40.0);
    
    return currents;
}
