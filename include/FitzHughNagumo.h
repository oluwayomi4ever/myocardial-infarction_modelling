#ifndef FITZHUGHNAGUMO_H
#define FITZHUGHNAGUMO_H

/**
 * @file FitzHughNagumo.h
 * @brief FitzHugh-Nagumo model for excitable media simulation
 */

#include <vector>
#include <functional>
#include <string>

class FitzHughNagumo {
public:
    /**
     * @brief Constructor for FitzHugh-Nagumo model
     * @param width Width of the simulation grid
     * @param height Height of the simulation grid
     * @param dt Time step
     */
    FitzHughNagumo(int width, int height, double dt = 0.01);
    
    /**
     * @brief Destructor
     */
    ~FitzHughNagumo();
    
    /**
     * @brief Initialize the simulation with default parameters
     */
    void initialize();
    
    /**
     * @brief Set model parameters
     * @param a Parameter a
     * @param b Parameter b
     * @param c Parameter c
     * @param d Parameter d
     */
    void setParameters(double a, double b, double c, double d);
    
    /**
     * @brief Set diffusion coefficients
     * @param du Diffusion coefficient for u variable
     * @param dv Diffusion coefficient for v variable
     */
    void setDiffusionCoefficients(double du, double dv);
    
    /**
     * @brief Set initial conditions
     * @param u_init Initial values for u variable
     * @param v_init Initial values for v variable
     */
    void setInitialConditions(const std::vector<std::vector<double>>& u_init,
                             const std::vector<std::vector<double>>& v_init);
    
    /**
     * @brief Add stimulus at specific location
     * @param x X coordinate
     * @param y Y coordinate
     * @param strength Stimulus strength
     * @param duration Stimulus duration
     */
    void addStimulus(int x, int y, double strength, double duration);
    
    /**
     * @brief Run one time step of the simulation
     */
    void step();
    
    /**
     * @brief Run simulation for specified number of steps
     * @param steps Number of time steps to run
     */
    void run(int steps);
    
    /**
     * @brief Get current u variable values
     * @return 2D grid of u values
     */
    const std::vector<std::vector<double>>& getU() const;
    
    /**
     * @brief Get current v variable values
     * @return 2D grid of v values
     */
    const std::vector<std::vector<double>>& getV() const;
    
    /**
     * @brief Get current simulation time
     * @return Current time
     */
    double getTime() const;
    
    /**
     * @brief Save current state to file
     * @param filename Output filename
     * @return true if successful, false otherwise
     */
    bool saveState(const std::string& filename) const;
    
    /**
     * @brief Load state from file
     * @param filename Input filename
     * @return true if successful, false otherwise
     */
    bool loadState(const std::string& filename);

private:
    int width_;          ///< Width of the simulation grid
    int height_;         ///< Height of the simulation grid
    double dt_;          ///< Time step
    double time_;        ///< Current simulation time
    
    // Model parameters
    double a_, b_, c_, d_;
    double du_, dv_;     ///< Diffusion coefficients
    
    // State variables
    std::vector<std::vector<double>> u_;  ///< Fast variable (membrane potential)
    std::vector<std::vector<double>> v_;  ///< Slow variable (recovery)
    std::vector<std::vector<double>> u_new_;  ///< Temporary storage for u
    std::vector<std::vector<double>> v_new_;  ///< Temporary storage for v
    
    // Stimulus
    std::vector<std::vector<double>> stimulus_;
    
    /**
     * @brief Apply diffusion operator
     * @param grid Input grid
     * @param coeff Diffusion coefficient
     * @param result Output grid
     */
    void applyDiffusion(const std::vector<std::vector<double>>& grid,
                       double coeff,
                       std::vector<std::vector<double>>& result);
    
    /**
     * @brief Apply reaction terms
     * @param u_val Current u value
     * @param v_val Current v value
     * @param stim_val Stimulus value
     * @return Reaction term for u
     */
    double reactionU(double u_val, double v_val, double stim_val);
    
    /**
     * @brief Apply reaction terms for v
     * @param u_val Current u value
     * @param v_val Current v value
     * @return Reaction term for v
     */
    double reactionV(double u_val, double v_val);
    
    /**
     * @brief Check if coordinates are within bounds
     * @param x X coordinate
     * @param y Y coordinate
     * @return true if coordinates are valid
     */
    bool isValidCoordinate(int x, int y) const;
};

#endif // FITZHUGHNAGUMO_H
