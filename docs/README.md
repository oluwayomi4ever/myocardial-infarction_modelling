# MI Modeling C++ Project

## Overview

This project provides open-source software tools for Myocardial Infarction (MI) modeling using computational approaches. The software implements various cardiac electrophysiology models, differential equation solvers, and validation frameworks for MI research.

## Project Structure

```
MI_Modeling_Cpp_Project/
├── CMakeLists.txt                 # Build configuration
├── include/                       # Header files
│   ├── DTM.h                     # Differential Transform Method solver
│   ├── FitzHughNagumo.h          # FitzHugh-Nagumo excitable media model
│   ├── CardiacElectrophysiology.h # Advanced cardiac electrophysiology models
│   ├── DataProcessor.h           # Clinical data preprocessing tools
│   └── ValidationFramework.h     # Model validation and comparison tools
├── src/                          # Source files
│   ├── main.cpp                  # Main application entry point
│   ├── DTM.cpp                   # DTM implementation
│   ├── FitzHughNagumo.cpp        # FitzHugh-Nagumo implementation
│   ├── CardiacElectrophysiology.cpp
│   ├── DataProcessor.cpp
│   └── ValidationFramework.cpp
├── data/                         # Data storage directory
├── tests/                        # Unit tests
└── docs/                         # Documentation
    ├── README.md
    ├── LITERATURE_REVIEW.md
    └── VALIDATION_REPORT.md
```

## Features

### Core Components

1. **Differential Transform Method (DTM)**
   - Numerical solver for differential equations
   - Optimized for cardiac electrophysiology problems
   - Support for various boundary conditions

2. **Cardiac Electrophysiology Models**
   - FitzHugh-Nagumo model (simplified excitable media)
   - Luo-Rudy dynamic model
   - Ten Tusscher human ventricular model
   - Support for different cell types and MI regions

3. **Clinical Data Processing**
   - ECG signal processing and analysis
   - MRI cardiac image processing
   - Echocardiogram analysis
   - Data integration and preprocessing pipelines

4. **Validation Framework**
   - Statistical validation metrics (RMSE, MAE, R²)
   - Sensitivity analysis tools
   - Clinical data comparison
   - Uncertainty quantification

## Build Instructions

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16 or higher
- Optional: OpenMP for parallel processing

### Building

```bash
# Clone the repository
git clone <repository-url>
cd MI_Modeling_Cpp_Project

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)

# Run tests (optional)
make test
```

### Building with specific options

```bash
# Build with OpenMP support
cmake -DENABLE_OPENMP=ON ..

# Build in Debug mode
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build with custom install prefix
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
```

## Usage

### Basic Usage

```bash
# Run with default demo
./MI_Modeling_Cpp_Project

# Run FitzHugh-Nagumo simulation
./MI_Modeling_Cpp_Project --fhn 100 100 1000

# Process DTM data
./MI_Modeling_Cpp_Project --dtm data/terrain.dat

# Show help
./MI_Modeling_Cpp_Project --help
```

### Programmatic Usage

```cpp
#include "CardiacElectrophysiology.h"
#include "ValidationFramework.h"

// Create Luo-Rudy model
LuoRudyModel model(100, 100, 0.01);
model.initialize();
model.setCellType("normal");

// Set MI region
std::vector<std::vector<bool>> mi_region(100, std::vector<bool>(100, false));
// Mark infarcted region
for (int i = 40; i < 60; ++i) {
    for (int j = 40; j < 60; ++j) {
        mi_region[i][j] = true;
    }
}
model.setMIRegion(mi_region);

// Run simulation
model.run(1000);

// Validate results
ModelValidator validator;
validator.addValidationData("clinical", clinical_data);
auto results = validator.validateModel(model.getMembranePotential(), "clinical");
```

## Research Applications

### Process Flow Implementation

This software implements the complete MI modeling research process flow:

1. **Literature Review & Background** ✅
   - Comprehensive documentation of models and methods
   - Reference implementations of established models

2. **Core Software Development** ✅
   - Modular architecture for MI modeling tools
   - DTM algorithms for differential equation solving
   - Baseline MI models (FitzHugh-Nagumo, Luo-Rudy, Ten Tusscher)

3. **Model Development & Refinement** ✅
   - Mathematical models for electrophysiology and tissue damage
   - DTM application to solve cardiac models
   - Parameter optimization and computational efficiency

4. **Data Integration** ✅
   - Clinical data preprocessing (ECG, MRI, Echo)
   - Structured data integration pipelines
   - Dataset-to-model parameter mapping

5. **Model Validation** ✅
   - Comparison with clinical/experimental data
   - Validation metrics and sensitivity testing
   - Model calibration and adjustment tools

## Contributing

### Development Guidelines

1. Follow C++17 standards and best practices
2. Use consistent naming conventions
3. Document all public interfaces
4. Write unit tests for new functionality
5. Update documentation for new features

### Code Style

- Use 4 spaces for indentation
- Maximum line length: 100 characters
- Use camelCase for functions and variables
- Use PascalCase for classes and types
- Include comprehensive comments for complex algorithms

### Testing

```bash
# Run all tests
cd build
make test

# Run specific test
./tests/test_cardiac_electrophysiology

# Run with verbose output
ctest --verbose
```

## Documentation

- [Literature Review](docs/LITERATURE_REVIEW.md) - Comprehensive review of MI modeling literature
- [Validation Report](docs/VALIDATION_REPORT.md) - Model validation results and comparisons
- [API Documentation](docs/API.md) - Detailed API reference
- [Examples](docs/EXAMPLES.md) - Usage examples and tutorials

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Citation

If you use this software in your research, please cite:

```bibtex
@software{mi_modeling_cpp,
  title={Open-Source Software Tools for Myocardial Infarction Modeling},
  author={[Your Name]},
  year={2024},
  url={https://github.com/your-repo/mi-modeling-cpp}
}
```

## Contact

- Project Lead: [Your Name] ([your.email@domain.com])
- Repository: [GitHub Repository URL]
- Issues: [GitHub Issues URL]

## Acknowledgments

- Cardiac electrophysiology community for model formulations
- Open-source software contributors
- Clinical collaborators for data and validation

