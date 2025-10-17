# MI Modeling Platform - Web Interface

A modern web-based user interface for the MI Modeling C++ Project that allows users to upload clinical data files and run cardiac electrophysiology simulations.

## Features

### 🎨 **Modern Web UI**
- **Responsive Design**: Works on desktop, tablet, and mobile devices
- **Drag & Drop Upload**: Easy file upload with visual feedback
- **Real-time Processing**: Live progress updates during simulation
- **Interactive Results**: Charts and visualizations of simulation results

### 📁 **File Upload Support**
- **ECG Data**: CSV files with time series data
- **MRI Data**: DICOM and HDF5 format support
- **Echocardiogram**: CSV files with wall motion data
- **DTM Data**: DAT and TXT format for differential transform method

### 🔬 **Simulation Models**
- **FitzHugh-Nagumo**: Simplified excitable media model
- **Luo-Rudy Dynamic**: Detailed guinea pig ventricular model
- **Ten Tusscher**: Human ventricular model with cell variants

### 📊 **Results & Visualization**
- **Real-time Charts**: Interactive time series plots
- **Metrics Display**: Key cardiac parameters
- **Export Functionality**: Download results in JSON format
- **Detailed Reports**: Comprehensive simulation reports

## Quick Start

### Prerequisites
- Python 3.7 or higher
- pip package manager
- C++ compiler (for building the simulation engine)

### Installation & Setup

1. **Navigate to web directory**:
   ```bash
   cd web/
   ```

2. **Run the startup script**:
   ```bash
   ./start_server.sh
   ```
   
   This script will:
   - Create a Python virtual environment
   - Install required dependencies
   - Build the C++ simulation engine (if needed)
   - Start the web server

3. **Open your browser**:
   ```
   http://localhost:5000
   ```

### Manual Setup

If you prefer manual setup:

1. **Create virtual environment**:
   ```bash
   python3 -m venv venv
   source venv/bin/activate
   ```

2. **Install dependencies**:
   ```bash
   pip install -r requirements.txt
   ```

3. **Build C++ project**:
   ```bash
   cd ..
   mkdir -p build && cd build
   cmake ..
   make -j$(nproc)
   cd ../web
   ```

4. **Start server**:
   ```bash
   python3 server.py
   ```

## Usage Guide

### 1. Upload Data Files
- **Drag & Drop**: Drag files directly onto the upload area
- **Click to Browse**: Click "Choose Files" to select files
- **Supported Formats**: CSV, DAT, TXT, H5, HDF5, DCM, DICOM
- **File Preview**: Click the eye icon to preview uploaded files

### 2. Configure Simulation Parameters
- **Model Type**: Choose between FitzHugh-Nagumo, Luo-Rudy, or Ten Tusscher
- **Grid Dimensions**: Set width and height of the computational grid
- **Time Step**: Specify the simulation time step in milliseconds
- **Simulation Steps**: Set the number of time steps to run
- **MI Region**: Enable/disable myocardial infarction region modeling

### 3. Run Simulation
- Click "Run Simulation" to start the analysis
- Monitor progress in real-time
- View processing steps and completion status

### 4. View Results
- **Metrics**: Key cardiac parameters (APD, conduction velocity, EF, etc.)
- **Charts**: Interactive time series plots
- **Detailed Results**: Click "View Detailed Results" for comprehensive report
- **Export**: Download results in JSON format

## API Endpoints

The web server provides a REST API for programmatic access:

### Upload Files
```
POST /api/upload
Content-Type: multipart/form-data

Returns: List of uploaded files with processing results
```

### Run Simulation
```
POST /api/simulate
Content-Type: application/json

Body: {
  "parameters": {
    "modelType": "fitzhugh-nagumo",
    "gridWidth": 100,
    "gridHeight": 100,
    "timeStep": 0.01,
    "simulationSteps": 1000,
    "includeMI": true
  },
  "files": ["file1.csv", "file2.dat"]
}

Returns: Simulation results and metrics
```

### Health Check
```
GET /api/health

Returns: Server status and version information
```

### Get Results
```
GET /api/results/<filename>

Returns: Simulation results file
```

## File Format Guidelines

### ECG Data (CSV)
```csv
time,lead1,lead2,lead3
0.000,-85.2,-85.1,-85.3
0.001,-85.1,-85.0,-85.2
...
```

### DTM Data (DAT/TXT)
```
100 100 1.0
0.0 1.2 2.3 4.5
1.1 2.2 3.3 5.4
...
```

### MRI Data (DICOM)
- Standard DICOM format
- Cardiac MRI sequences
- Late gadolinium enhancement (LGE) preferred

## Customization

### Adding New File Types
1. Update `ALLOWED_EXTENSIONS` in `server.py`
2. Add processing function (e.g., `process_new_type_data()`)
3. Update `get_file_type()` mapping
4. Add file type icon in `script.js`

### Customizing UI
- **Styling**: Modify `styles.css`
- **Layout**: Edit `index.html`
- **Functionality**: Update `script.js`

### Adding New Models
1. Implement C++ model in the main project
2. Add command-line interface support
3. Update web server to call new model
4. Add UI controls for new parameters

## Troubleshooting

### Common Issues

**Server won't start**:
- Check Python version: `python3 --version`
- Verify virtual environment: `source venv/bin/activate`
- Install dependencies: `pip install -r requirements.txt`

**C++ executable not found**:
- Build the project: `cd .. && mkdir build && cd build && cmake .. && make`
- Check executable path in `server.py`

**File upload fails**:
- Check file size (max 100MB)
- Verify file format is supported
- Check upload directory permissions

**Simulation errors**:
- Verify C++ executable works: `./MI_Modeling_Cpp_Project --help`
- Check simulation parameters
- Review server logs for error messages

### Logs
- Server logs are displayed in the terminal
- Upload and processing logs are written to console
- Simulation errors are captured and returned to UI

## Development

### Project Structure
```
web/
├── index.html          # Main UI page
├── styles.css          # CSS styling
├── script.js           # Frontend JavaScript
├── server.py           # Flask backend server
├── requirements.txt    # Python dependencies
├── start_server.sh     # Startup script
├── README.md          # This file
├── uploads/           # Uploaded files (created automatically)
└── results/           # Simulation results (created automatically)
```

### Contributing
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This web interface is part of the MI Modeling C++ Project and is licensed under the MIT License.

## Support

For issues and questions:
- Check the troubleshooting section above
- Review the main project documentation
- Create an issue on the GitHub repository
- Contact the development team


