# MI Modeling C++ Project - Usage Guide

## Quick Start

### 1. Build the Project
```bash
cd /home/ned/projects/cpp/mcmodel
./build.sh
```

### 2. Run Tests
```bash
cd build
./simple_tests
```

### 3. Test Data Processing
```bash
cd build
./data_test
```

## Data Testing and Usage

### Sample Data Files

The project includes sample data files in the `data/` directory:

- **`sample_ecg_data.txt`** - 12-lead ECG data (1000 samples)
- **`sample_mri_data.txt`** - 100x100 cardiac MRI image data
- **`sample_echo_data.txt`** - Echocardiogram data (15 frames)
- **`config.txt`** - Data configuration file

### Data File Formats

#### ECG Data Format
```
sampling_rate: 1000
num_leads: 12
num_samples: 1000
[12-lead ECG data - one sample per line]
```

#### MRI Data Format
```
width height
[pixel values - space-separated, one row per line]
```

#### Echo Data Format
```
[frame data - one frame per line, space-separated values]
```

### Testing with Data

#### 1. Individual Data Processing Tests
```bash
cd build
./data_test
```

This will test:
- ✅ ECG data loading and processing
- ✅ QRS parameter extraction
- ✅ Arrhythmia detection
- ✅ MRI image processing
- ✅ Tissue segmentation
- ✅ Wall thickness analysis
- ✅ Perfusion mapping
- ✅ Echo data processing
- ✅ Ejection fraction calculation
- ✅ Wall motion analysis

#### 2. Data Integration Testing
The data test also demonstrates:
- Multi-modal data integration
- Model parameter generation
- Tissue map creation
- Clinical validation

#### 3. Validation Framework Testing
- Statistical validation metrics (RMSE, MAE, R²)
- Clinical data comparison
- Model validation reports

### Using Your Own Data

#### 1. Prepare Your Data Files

**For ECG Data:**
```bash
# Create your ECG file in the format:
echo "sampling_rate: 1000" > my_ecg.txt
echo "num_leads: 12" >> my_ecg.txt
echo "num_samples: 1000" >> my_ecg.txt
# Add your 12-lead ECG data (one sample per line)
```

**For MRI Data:**
```bash
# Create your MRI file in the format:
echo "width height" > my_mri.txt
# Add your pixel data (one row per line)
```

**For Echo Data:**
```bash
# Create your echo file with frame data
# One frame per line, space-separated values
```

#### 2. Update Configuration
Edit `data/config.txt`:
```
ecg data/my_ecg.txt
mri data/my_mri.txt
echo data/my_echo.txt
```

#### 3. Run Data Tests
```bash
cd build
./data_test
```

### Advanced Usage Examples

#### 1. FitzHugh-Nagumo Simulation with Different Parameters
```bash
cd build
./MI_Modeling_Cpp_Project --fhn 100 100 2000
```

#### 2. DTM Data Processing
```bash
cd build
# Create sample DTM data first
echo "50 50 1.0" > my_dtm.txt
# Add elevation data (50x50 grid)
./MI_Modeling_Cpp_Project --dtm my_dtm.txt
```

#### 3. Custom Data Processing Pipeline
```cpp
#include "DataProcessor.h"
#include "ValidationFramework.h"

int main() {
    // Load and process ECG data
    ECGProcessor ecg;
    ecg.loadData("data/my_ecg.txt");
    ecg.processData();
    
    // Extract features
    auto params = ecg.extractQRSParameters();
    auto arrhythmia = ecg.detectArrhythmias();
    
    // Validate against clinical data
    ModelValidator validator;
    // ... validation code
    
    return 0;
}
```

### Generated Output Files

After running data tests, check `build/data/` for:

- **`processed_ecg.txt`** - Processed ECG data
- **`processed_mri.txt`** - Processed MRI data  
- **`processed_echo.txt`** - Processed echo data
- **`validation_report.txt`** - Validation metrics report
- **`clinical_report.txt`** - Clinical comparison report
- **`fhn_final_state.dat`** - FitzHugh-Nagumo simulation results

### Performance Testing

#### 1. Benchmark Different Grid Sizes
```bash
cd build
time ./MI_Modeling_Cpp_Project --fhn 50 50 1000
time ./MI_Modeling_Cpp_Project --fhn 100 100 1000
time ./MI_Modeling_Cpp_Project --fhn 200 200 1000
```

#### 2. Memory Usage Monitoring
```bash
cd build
valgrind --tool=massif ./MI_Modeling_Cpp_Project --fhn 100 100 1000
```

### Troubleshooting

#### Common Issues:

1. **"Cannot create file data/..."**
   ```bash
   mkdir -p build/data
   ```

2. **"Cannot open file data/..."**
   - Check file paths in `data/config.txt`
   - Ensure data files exist in correct format

3. **Build errors**
   ```bash
   # Clean and rebuild
   rm -rf build/*
   ./build.sh
   ```

4. **Permission errors**
   ```bash
   chmod +x build.sh
   chmod +x build/*
   ```

### Data Validation

#### 1. Check Data Integrity
```bash
# Verify ECG data format
head -5 data/sample_ecg_data.txt

# Verify MRI data dimensions
head -1 data/sample_mri_data.txt

# Count echo frames
wc -l data/sample_echo_data.txt
```

#### 2. Validate Processing Results
```bash
cd build
./data_test > results.log 2>&1
grep "✅\|❌" results.log
```

### Integration with External Tools

#### 1. Python Integration
```python
import subprocess
import numpy as np

# Run C++ simulation
result = subprocess.run(['./MI_Modeling_Cpp_Project', '--fhn', '100', '100', '1000'], 
                       capture_output=True, text=True)

# Process results
print(result.stdout)
```

#### 2. MATLAB Integration
```matlab
% Run C++ program
[status, output] = system('./MI_Modeling_Cpp_Project --fhn 100 100 1000');

% Process output
disp(output);
```

### Best Practices

1. **Data Organization**
   - Keep raw data in `data/` directory
   - Use descriptive filenames
   - Document data formats and sources

2. **Testing Strategy**
   - Run unit tests before data processing
   - Validate data formats before processing
   - Check output files for expected results

3. **Performance Optimization**
   - Use appropriate grid sizes for your needs
   - Monitor memory usage for large datasets
   - Consider parallel processing for large simulations

4. **Error Handling**
   - Check return values from data loading functions
   - Validate input data formats
   - Use try-catch blocks for robust error handling

### Getting Help

1. **Check the documentation:**
   ```bash
   cat docs/README.md
   cat docs/LITERATURE_REVIEW.md
   ```

2. **Run help commands:**
   ```bash
   ./MI_Modeling_Cpp_Project --help
   ```

3. **Examine test outputs:**
   ```bash
   ./simple_tests
   ./data_test
   ```

4. **Check generated reports:**
   ```bash
   cat build/data/validation_report.txt
   cat build/data/clinical_report.txt
   ```
