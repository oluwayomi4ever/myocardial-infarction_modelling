#!/bin/bash

# MI Modeling WebAssembly Build Script
# This script compiles C++ code to WebAssembly using Emscripten

echo "🧬 MI Modeling - WebAssembly Build Script"
echo "=========================================="

# Check if Emscripten is installed
if ! command -v emcc &> /dev/null; then
    echo "❌ Emscripten not found!"
    echo "📥 Installing Emscripten..."
    
    # Clone and install Emscripten
    if [ ! -d "emsdk" ]; then
        git clone https://github.com/emscripten-core/emsdk.git
    fi
    
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh
    cd ..
    
    echo "✅ Emscripten installed successfully!"
else
    echo "✅ Emscripten found!"
fi

# Create output directory
mkdir -p dist

echo "🔨 Compiling C++ to WebAssembly..."

# Compile main WebAssembly module
emcc -O3 \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_run_fitzhugh_nagumo_simulation", "_run_dtm_simulation"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "addFunction", "removeFunction"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='MIModelingWASM' \
    -s ENVIRONMENT='web' \
    -s NO_EXIT_RUNTIME=1 \
    -s ASSERTIONS=1 \
    -s DISABLE_EXCEPTION_CATCHING=0 \
    --bind \
    -I../include \
    ../src/FitzHughNagumo.cpp \
    ../src/DTM.cpp \
    mi_modeling_wasm.cpp \
    -o dist/mi_modeling.js

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "✅ WebAssembly compilation successful!"
    echo "📁 Output files:"
    echo "   - dist/mi_modeling.js"
    echo "   - dist/mi_modeling.wasm"
    
    # Display file sizes
    if [ -f "dist/mi_modeling.js" ]; then
        echo "📊 JavaScript bundle size: $(du -h dist/mi_modeling.js | cut -f1)"
    fi
    
    if [ -f "dist/mi_modeling.wasm" ]; then
        echo "📊 WebAssembly binary size: $(du -h dist/mi_modeling.wasm | cut -f1)"
    fi
    
    echo ""
    echo "🚀 WebAssembly module ready for use!"
    echo "📖 To use in your HTML:"
    echo "   <script src='dist/mi_modeling.js'></script>"
    echo "   <script>"
    echo "     MIModelingWASM().then(Module => {"
    echo "       const model = new Module.FitzHughNagumo(100, 100);"
    echo "       model.initialize();"
    echo "       model.run(1000);"
    echo "     });"
    echo "   </script>"
    
else
    echo "❌ WebAssembly compilation failed!"
    echo "🔍 Check the error messages above for details."
    exit 1
fi

# Create a smaller optimized version for production
echo ""
echo "🔨 Creating optimized production build..."

emcc -O3 \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_run_fitzhugh_nagumo_simulation", "_run_dtm_simulation"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='MIModelingWASM' \
    -s ENVIRONMENT='web' \
    -s NO_EXIT_RUNTIME=1 \
    -s ASSERTIONS=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    --bind \
    -I../include \
    ../src/FitzHughNagumo.cpp \
    ../src/DTM.cpp \
    mi_modeling_wasm.cpp \
    -o dist/mi_modeling.min.js

if [ $? -eq 0 ]; then
    echo "✅ Optimized build created!"
    echo "📊 Optimized JavaScript size: $(du -h dist/mi_modeling.min.js | cut -f1)"
else
    echo "⚠️ Optimized build failed, but debug build succeeded"
fi

echo ""
echo "🎉 Build process completed!"
echo "📂 All files are in the 'dist/' directory"
echo "🌐 You can now use these files in your web application"

