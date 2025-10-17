#!/bin/bash

# MI Modeling Platform - Web Server Startup Script

echo "MI Modeling Platform - Starting Web Server"
echo "=========================================="

# Check if Python is installed
if ! command -v python3 &> /dev/null; then
    echo "Error: Python3 is not installed"
    exit 1
fi

# Check if pip is installed
if ! command -v pip3 &> /dev/null; then
    echo "Error: pip3 is not installed"
    exit 1
fi

# Create virtual environment if it doesn't exist
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
fi

# Activate virtual environment
echo "Activating virtual environment..."
source venv/bin/activate

# Install dependencies
echo "Installing Python dependencies..."
pip install -r requirements.txt

# Create necessary directories
mkdir -p uploads results

# Check if C++ executable exists
if [ ! -f "../MI_Modeling_Cpp_Project" ]; then
    echo "Warning: C++ executable not found. Building..."
    cd ..
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    cd ../web
fi

# Start the web server
echo "Starting web server on http://localhost:5000"
echo "Press Ctrl+C to stop the server"
echo ""

python3 server.py


