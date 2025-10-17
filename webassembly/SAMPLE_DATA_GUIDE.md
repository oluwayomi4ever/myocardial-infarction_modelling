# Sample Data Testing Guide

## 📁 Available Sample Data Files

I've created several sample clinical data files for you to test with the MI Modeling application:

### 1. **ECG Data Files**
- **`patient_001_ecg.csv`** - Normal ECG with standard 12-lead data
- **`patient_002_mi_ecg.csv`** - ECG showing signs of myocardial infarction (elevated ST segments, T-wave inversions)

### 2. **Echocardiogram Data**
- **`patient_003_echo.json`** - Complete echocardiogram report with:
  - Left ventricular function (EF: 45.2%)
  - Diastolic function parameters
  - Wall motion analysis
  - Valve assessments

### 3. **MRI Data**
- **`patient_004_mri.txt`** - Cardiac MRI report with:
  - Volumetric measurements
  - Perfusion study results
  - Delayed enhancement findings
  - Tissue characterization

### 4. **CT Data**
- **`patient_005_ct.csv`** - Cardiac CT scan data with:
  - Wall thickness measurements
  - Calcification scores
  - Contrast enhancement values

## 🧪 How to Test with Sample Data

### **Step 1: Access the Application**
1. Open your browser and go to: `http://localhost:8080`
2. Wait for the WebAssembly module to load (green status message)

### **Step 2: Upload Sample Files**
1. **Click "Choose Files" or drag and drop** files from the `sample_data/` folder
2. **Recommended test combinations:**
   - **Single file test**: Upload just `patient_002_mi_ecg.csv`
   - **Multi-file test**: Upload `patient_002_mi_ecg.csv` + `patient_003_echo.json`
   - **Complete dataset**: Upload all 4 files together

### **Step 3: Configure Simulation Parameters**
**For Basic Testing:**
- Grid Size: 50x50
- Time Step: 0.01
- Simulation Steps: 1000
- Stimulus: Center position (25, 25)

**For Classification Metrics Testing:**
- ✅ **Enable "Training Mode (Epoch-based)"**
- Number of Epochs: 5-10
- Steps per Epoch: 200-500

### **Step 4: Run the Simulation**
1. Click **"Run C++ Simulation"**
2. Watch the progress updates
3. Wait for completion (should take 10-30 seconds)

## 📊 Expected Results

### **Normal ECG (patient_001_ecg.csv):**
- Membrane potential: -80 to -70 mV (normal range)
- No significant abnormalities
- Classification metrics: Lower accuracy (normal tissue)

### **MI ECG (patient_002_mi_ecg.csv):**
- Membrane potential: -70 to -60 mV (depolarized)
- ST elevation patterns
- Classification metrics: Higher accuracy (MI detection)

### **With Training Mode Enabled:**
You'll see:
- **Epoch Results Table**: Progress for each training epoch
- **Classification Metrics**: F1 score, precision, recall, accuracy
- **Confusion Matrix Chart**: Visual representation
- **Metrics Evolution Chart**: How metrics improve over epochs

## 🔬 Advanced Testing Scenarios

### **Scenario 1: MI Detection**
```
Files: patient_002_mi_ecg.csv + patient_003_echo.json
Training Mode: Enabled
Epochs: 10
Expected: High accuracy in detecting MI regions
```

### **Scenario 2: Normal vs Abnormal Comparison**
```
Test 1: Upload patient_001_ecg.csv (normal)
Test 2: Upload patient_002_mi_ecg.csv (MI)
Compare: Classification metrics differences
```

### **Scenario 3: Multi-Modal Analysis**
```
Files: All 4 sample files
Training Mode: Enabled
Epochs: 15
Expected: Comprehensive analysis with high accuracy
```

## 🎯 Key Metrics to Watch

### **Classification Performance:**
- **Accuracy**: Should be >70% for MI detection
- **F1 Score**: Should be >0.6 for good performance
- **Precision**: True positive rate
- **Recall**: Sensitivity to MI detection

### **Training Progress:**
- **Epoch Results**: Each epoch should show improving metrics
- **Best Accuracy**: Highest accuracy achieved
- **Average Accuracy**: Overall training performance

## 🚨 Troubleshooting

### **If files won't upload:**
- Check file paths in `sample_data/` folder
- Ensure files are not corrupted
- Try uploading one file at a time

### **If simulation fails:**
- Check WebAssembly status (should be green)
- Try smaller grid sizes (30x30)
- Reduce number of epochs

### **If metrics don't display:**
- Ensure "Training Mode" is enabled
- Check browser console for errors
- Try refreshing the page

## 📈 Sample Expected Output

**For patient_002_mi_ecg.csv with 10 epochs:**
```
Epoch 1: Accuracy: 72.5%, F1: 0.68
Epoch 2: Accuracy: 75.2%, F1: 0.71
Epoch 3: Accuracy: 78.1%, F1: 0.74
...
Epoch 10: Accuracy: 84.7%, F1: 0.82

Final Classification Metrics:
- Accuracy: 84.7%
- Precision: 82.3%
- Recall: 87.1%
- F1 Score: 84.6%
```

## 🎉 Success Indicators

✅ **Application working correctly when:**
- Files upload successfully
- Simulation completes without errors
- Classification metrics display
- Charts render properly
- Training progress shows improvement over epochs

Happy testing! 🧬📊
