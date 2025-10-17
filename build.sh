#!/bin/bash

# Simple build script for MI Modeling C++ Project
echo "Building MI Modeling C++ Project..."

# Create build directory
mkdir -p build
cd build

# Compile the main project
echo "Compiling main executable..."
g++ -std=c++17 -Wall -Wextra -O2 -I../include \
    ../src/main.cpp \
    ../src/DTM.cpp \
    ../src/FitzHughNagumo.cpp \
    ../src/CardiacElectrophysiology.cpp \
    ../src/DataProcessor.cpp \
    ../src/ValidationFramework.cpp \
    -o MI_Modeling_Cpp_Project

if [ $? -eq 0 ]; then
    echo "Main executable built successfully!"
else
    echo "Error building main executable"
    exit 1
fi

# Compile simple tests
echo "Compiling tests..."
g++ -std=c++17 -Wall -Wextra -O2 -I../include \
    ../tests/simple_test_main.cpp \
    ../src/DTM.cpp \
    ../src/FitzHughNagumo.cpp \
    ../src/ValidationFramework.cpp \
    -o simple_tests

if [ $? -eq 0 ]; then
    echo "Tests compiled successfully!"
else
    echo "Error building tests"
    exit 1
fi

# Compile data testing program
echo "Compiling data testing program..."
g++ -std=c++17 -Wall -Wextra -O2 -I../include \
    ../src/data_test.cpp \
    ../src/DataProcessor.cpp \
    ../src/ValidationFramework.cpp \
    -o data_test

if [ $? -eq 0 ]; then
    echo "Data testing program compiled successfully!"
else
    echo "Error building data testing program"
    exit 1
fi

echo "Build completed successfully!"
echo ""
echo "Available programs:"
echo "  ./MI_Modeling_Cpp_Project    - Main MI modeling application"
echo "  ./simple_tests               - Unit tests"
echo "  ./data_test                  - Data processing and validation tests"
echo ""
echo "Usage examples:"
echo "  ./MI_Modeling_Cpp_Project --help                    # Show help"
echo "  ./MI_Modeling_Cpp_Project                          # Run demo"
echo "  ./MI_Modeling_Cpp_Project --fhn 100 100 1000       # Run FitzHugh-Nagumo simulation"
echo "  ./MI_Modeling_Cpp_Project --dtm data/terrain.dat   # Process DTM data"
echo "  ./simple_tests                                      # Run unit tests"
echo "  ./data_test                                         # Test data processing"
