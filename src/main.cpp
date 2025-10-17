#include <iostream>
#include <string>
#include <chrono>
#include <cmath>
#include "DTM.h"
#include "FitzHughNagumo.h"

/**
 * @file main.cpp
 * @brief Main entry point for MI_Modeling_Cpp_Project
 */

void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --dtm <filename>     Load and process DTM data\n";
    std::cout << "  --fhn <width> <height> [steps]  Run FitzHugh-Nagumo simulation\n";
    std::cout << "  --help              Show this help message\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " --dtm terrain.dat\n";
    std::cout << "  " << programName << " --fhn 100 100 1000\n";
}

void runDTMDemo() {
    std::cout << "\n=== DTM (Digital Terrain Model) Demo ===\n";
    
    // Create a simple DTM grid
    DTM dtm(50, 50, 10.0);
    
    // Fill with some sample elevation data (simple hill)
    for (int x = 0; x < 50; ++x) {
        for (int y = 0; y < 50; ++y) {
            double center_x = 25.0;
            double center_y = 25.0;
            double distance = std::sqrt((x - center_x) * (x - center_x) + (y - center_y) * (y - center_y));
            double elevation = 100.0 * std::exp(-distance / 10.0);
            dtm.setElevation(x, y, elevation);
        }
    }
    
    // Calculate and display some terrain properties
    std::cout << "DTM Grid: " << dtm.getDimensions().first << "x" << dtm.getDimensions().second << "\n";
    std::cout << "Cell Size: " << dtm.getCellSize() << "m\n";
    std::cout << "Center elevation: " << dtm.getElevation(25, 25) << "m\n";
    std::cout << "Center slope: " << dtm.calculateSlope(25, 25) << " degrees\n";
    std::cout << "Center aspect: " << dtm.calculateAspect(25, 25) << " degrees\n";
    
    // Save to file
    if (dtm.saveToFile("data/sample_dtm.dat")) {
        std::cout << "DTM data saved to data/sample_dtm.dat\n";
    } else {
        std::cout << "Failed to save DTM data\n";
    }
}

void runFitzHughNagumoDemo(int width, int height, int steps) {
    std::cout << "\n=== FitzHugh-Nagumo Model Demo ===\n";
    
    // Create simulation
    FitzHughNagumo fhn(width, height, 0.01);
    fhn.initialize();
    
    // Set model parameters (typical values for excitable media)
    fhn.setParameters(0.1, 0.5, 1.0, 0.0);
    fhn.setDiffusionCoefficients(0.1, 0.0);
    
    // Add initial stimulus in center
    int center_x = width / 2;
    int center_y = height / 2;
    fhn.addStimulus(center_x, center_y, 1.0, 10.0);
    
    std::cout << "Grid size: " << width << "x" << height << "\n";
    std::cout << "Running " << steps << " time steps...\n";
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Run simulation
    fhn.run(steps);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Simulation completed in " << duration.count() << " ms\n";
    std::cout << "Final time: " << fhn.getTime() << " s\n";
    
    // Save final state
    if (fhn.saveState("data/fhn_final_state.dat")) {
        std::cout << "Final state saved to data/fhn_final_state.dat\n";
    } else {
        std::cout << "Failed to save final state\n";
    }
}

int main(int argc, char* argv[]) {
    std::cout << "MI Modeling C++ Project\n";
    std::cout << "=======================\n";
    
    if (argc < 2) {
        std::cout << "No arguments provided. Running demo mode...\n";
        
        // Run both demos
        runDTMDemo();
        runFitzHughNagumoDemo(50, 50, 1000);
        
        return 0;
    }
    
    std::string arg1(argv[1]);
    
    if (arg1 == "--help" || arg1 == "-h") {
        printUsage(argv[0]);
        return 0;
    }
    else if (arg1 == "--dtm") {
        if (argc < 3) {
            std::cerr << "Error: DTM filename required\n";
            printUsage(argv[0]);
            return 1;
        }
        
        std::string filename(argv[2]);
        DTM dtm(100, 100);  // Default size
        
        if (dtm.loadFromFile(filename)) {
            std::cout << "DTM loaded successfully from " << filename << "\n";
            std::cout << "Dimensions: " << dtm.getDimensions().first << "x" << dtm.getDimensions().second << "\n";
        } else {
            std::cerr << "Failed to load DTM from " << filename << "\n";
            return 1;
        }
    }
    else if (arg1 == "--fhn") {
        if (argc < 4) {
            std::cerr << "Error: Width and height required for FitzHugh-Nagumo simulation\n";
            printUsage(argv[0]);
            return 1;
        }
        
        int width = std::stoi(argv[2]);
        int height = std::stoi(argv[3]);
        int steps = (argc > 4) ? std::stoi(argv[4]) : 1000;
        
        if (width <= 0 || height <= 0 || steps <= 0) {
            std::cerr << "Error: Width, height, and steps must be positive integers\n";
            return 1;
        }
        
        runFitzHughNagumoDemo(width, height, steps);
    }
    else {
        std::cerr << "Error: Unknown argument " << arg1 << "\n";
        printUsage(argv[0]);
        return 1;
    }
    
    return 0;
}
