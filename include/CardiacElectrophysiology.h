#ifndef CARDIACELECTROPHYSIOLOGY_H
#define CARDIACELECTROPHYSIOLOGY_H

/**
 * @file CardiacElectrophysiology.h
 * @brief Cardiac electrophysiology models for MI modeling
 */

#include <vector>
#include <string>
#include <functional>
#include <map>

/**
 * @brief Base class for cardiac electrophysiology models
 */
class CardiacElectrophysiology {
public:
    /**
     * @brief Constructor
     * @param width Grid width
     * @param height Grid height
     * @param dt Time step
     */
    CardiacElectrophysiology(int width, int height, double dt = 0.01);
    
    virtual ~CardiacElectrophysiology() = default;
    
    /**
     * @brief Run one time step of the simulation
     */
    virtual void step() = 0;
    
    /**
     * @brief Run simulation for specified number of steps
     * @param steps Number of time steps
     */
    virtual void run(int steps);
    
    /**
     * @brief Get membrane potential (V)
     * @return 2D grid of membrane potential values
     */
    virtual const std::vector<std::vector<double>>& getMembranePotential() const = 0;
    
    /**
     * @brief Get current simulation time
     * @return Current time in milliseconds
     */
    double getTime() const { return time_; }
    
    /**
     * @brief Set tissue conductivity
     * @param conductivity Conductivity value (S/cm)
     */
    void setConductivity(double conductivity) { conductivity_ = conductivity; }
    
    /**
     * @brief Set MI region (damaged tissue)
     * @param mi_region 2D boolean grid indicating MI regions
     */
    void setMIRegion(const std::vector<std::vector<bool>>& mi_region);

protected:
    int width_, height_;
    double dt_, time_;
    double conductivity_;
    std::vector<std::vector<bool>> mi_region_;
    
    /**
     * @brief Apply diffusion operator for electrical propagation
     * @param grid Input grid
     * @param result Output grid
     */
    void applyDiffusion(const std::vector<std::vector<double>>& grid,
                       std::vector<std::vector<double>>& result);
};

/**
 * @brief Luo-Rudy dynamic model for cardiac electrophysiology
 */
class LuoRudyModel : public CardiacElectrophysiology {
public:
    LuoRudyModel(int width, int height, double dt = 0.01);
    ~LuoRudyModel() override;
    
    void step() override;
    const std::vector<std::vector<double>>& getMembranePotential() const override { return V_; }
    
    /**
     * @brief Set model parameters for different cell types
     * @param cell_type Type of cardiac cell (normal, ischemic, infarcted)
     */
    void setCellType(const std::string& cell_type);
    
    /**
     * @brief Get ionic currents
     * @return Map of current names to 2D grids
     */
    std::map<std::string, std::vector<std::vector<double>>> getIonicCurrents() const;

private:
    // State variables
    std::vector<std::vector<double>> V_;  // Membrane potential
    std::vector<std::vector<double>> m_, h_, j_;  // Sodium gate variables
    std::vector<std::vector<double>> xr_, xs_;    // Potassium gate variables
    std::vector<std::vector<double>> d_, f_, fca_; // Calcium gate variables
    std::vector<std::vector<double>> Cai_, CaSR_; // Calcium concentrations
    
    // Model parameters
    double GNa_, Gsi_, GK_, GK1_, Gb_, GCaL_;
    
    /**
     * @brief Calculate ionic currents
     * @param x X coordinate
     * @param y Y coordinate
     * @return Map of current values
     */
    std::map<std::string, double> calculateIonicCurrents(int x, int y);
};

/**
 * @brief Ten Tusscher model for human ventricular electrophysiology
 */
class TenTusscherModel : public CardiacElectrophysiology {
public:
    TenTusscherModel(int width, int height, double dt = 0.01);
    ~TenTusscherModel() override;
    
    void step() override;
    const std::vector<std::vector<double>>& getMembranePotential() const override { return V_; }
    
    /**
     * @brief Set model variant (epi, endo, mid)
     * @param variant Cell type variant
     */
    void setVariant(const std::string& variant);

private:
    // State variables (simplified for demonstration)
    std::vector<std::vector<double>> V_;  // Membrane potential
    std::vector<std::vector<double>> m_, h_, j_, oa_, oi_;  // Gate variables
    std::vector<std::vector<double>> d_, f_, fca_, u_, v_, w_;  // Additional gates
    std::vector<std::vector<double>> Cai_, CaSR_, CaSS_, Nai_, Ki_;  // Concentrations
    
    // Model parameters
    double GNa_, GCaL_, GKr_, GKs_, GK1_, Gto_, GNaCa_, GNaK_;
    
    /**
     * @brief Calculate ionic currents for Ten Tusscher model
     */
    std::map<std::string, double> calculateIonicCurrents(int x, int y);
};

#endif // CARDIACELECTROPHYSIOLOGY_H

