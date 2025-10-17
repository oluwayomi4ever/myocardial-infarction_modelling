#!/usr/bin/env python3
"""
MI Modeling Platform - Web Server
Handles file uploads and integrates with C++ models
"""

import os
import json
import subprocess
import tempfile
import shutil
from flask import Flask, request, jsonify, send_from_directory, render_template_string
from flask_cors import CORS
from werkzeug.utils import secure_filename
import numpy as np
import pandas as pd
from datetime import datetime
import logging

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = Flask(__name__)
CORS(app)

# Configuration
UPLOAD_FOLDER = 'uploads'
RESULTS_FOLDER = 'results'
ALLOWED_EXTENSIONS = {'csv', 'dat', 'txt', 'h5', 'hdf5', 'dcm', 'dicom'}
MAX_CONTENT_LENGTH = 100 * 1024 * 1024  # 100MB max file size

app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.config['RESULTS_FOLDER'] = RESULTS_FOLDER
app.config['MAX_CONTENT_LENGTH'] = MAX_CONTENT_LENGTH

# Create directories if they don't exist
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
os.makedirs(RESULTS_FOLDER, exist_ok=True)

def allowed_file(filename):
    """Check if file extension is allowed"""
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

def get_file_type(filename):
    """Determine file type based on extension"""
    ext = filename.rsplit('.', 1)[1].lower()
    type_map = {
        'csv': 'ecg',
        'dat': 'dtm',
        'txt': 'dtm',
        'h5': 'mri',
        'hdf5': 'mri',
        'dcm': 'mri',
        'dicom': 'mri'
    }
    return type_map.get(ext, 'unknown')

def process_ecg_data(filepath):
    """Process ECG data from CSV file"""
    try:
        df = pd.read_csv(filepath)
        logger.info(f"ECG data shape: {df.shape}")
        
        # Basic ECG analysis
        if len(df.columns) >= 2:
            time_col = df.columns[0]
            signal_col = df.columns[1]
            
            # Calculate basic metrics
            signal = df[signal_col].values
            metrics = {
                'duration': len(signal),
                'mean_value': float(np.mean(signal)),
                'std_value': float(np.std(signal)),
                'min_value': float(np.min(signal)),
                'max_value': float(np.max(signal)),
                'sampling_rate': 1000  # Assume 1kHz if not specified
            }
            
            # Detect R-peaks (simplified)
            threshold = np.mean(signal) + 2 * np.std(signal)
            r_peaks = np.where(signal > threshold)[0]
            metrics['r_peaks_count'] = len(r_peaks)
            
            return {'success': True, 'data': df.to_dict(), 'metrics': metrics}
        else:
            return {'success': False, 'error': 'Invalid ECG data format'}
            
    except Exception as e:
        logger.error(f"Error processing ECG data: {str(e)}")
        return {'success': False, 'error': str(e)}

def process_mri_data(filepath):
    """Process MRI data from DICOM or HDF5 file"""
    try:
        # For demo purposes, create mock MRI data
        # In a real implementation, you would use pydicom or h5py
        
        metrics = {
            'image_dimensions': [256, 256, 64],
            'voxel_size': [1.0, 1.0, 2.0],
            'scan_type': 'cardiac_mri',
            'contrast_enhanced': True,
            'slice_thickness': 2.0
        }
        
        return {'success': True, 'metrics': metrics}
        
    except Exception as e:
        logger.error(f"Error processing MRI data: {str(e)}")
        return {'success': False, 'error': str(e)}

def process_dtm_data(filepath):
    """Process DTM (Differential Transform Method) data"""
    try:
        # Read DTM data file
        with open(filepath, 'r') as f:
            lines = f.readlines()
        
        # Parse DTM format (assuming simple format)
        data = []
        for line in lines[1:]:  # Skip header
            if line.strip():
                values = [float(x) for x in line.strip().split()]
                data.append(values)
        
        if data:
            metrics = {
                'grid_size': len(data),
                'data_points': len(data[0]) if data else 0,
                'min_value': float(np.min(data)),
                'max_value': float(np.max(data)),
                'mean_value': float(np.mean(data))
            }
            return {'success': True, 'data': data, 'metrics': metrics}
        else:
            return {'success': False, 'error': 'No data found in file'}
            
    except Exception as e:
        logger.error(f"Error processing DTM data: {str(e)}")
        return {'success': False, 'error': str(e)}

def run_cpp_simulation(parameters, uploaded_files):
    """Run C++ simulation with uploaded files and parameters"""
    try:
        # Prepare simulation command
        cmd = [
            '../MI_Modeling_Cpp_Project',
            '--fhn',
            str(parameters['gridWidth']),
            str(parameters['gridHeight']),
            str(parameters['simulationSteps'])
        ]
        
        # Add model type specific parameters
        if parameters['modelType'] == 'luo-rudy':
            cmd.extend(['--model', 'luo-rudy'])
        elif parameters['modelType'] == 'ten-tusscher':
            cmd.extend(['--model', 'ten-tusscher'])
        
        # Run simulation
        logger.info(f"Running simulation: {' '.join(cmd)}")
        result = subprocess.run(cmd, capture_output=True, text=True, cwd='..')
        
        if result.returncode == 0:
            # Parse simulation output
            output = result.stdout
            logger.info(f"Simulation output: {output}")
            
            # Generate mock results for demo
            simulation_results = {
                'success': True,
                'actionPotentialDuration': 280 + np.random.normal(0, 10),
                'conductionVelocity': 0.45 + np.random.normal(0, 0.05),
                'ejectionFraction': 55 + np.random.normal(0, 5),
                'wallMotionScore': 1.2 + np.random.normal(0, 0.3),
                'simulation_time': 1.5,  # seconds
                'parameters_used': parameters
            }
            
            return simulation_results
        else:
            logger.error(f"Simulation failed: {result.stderr}")
            return {'success': False, 'error': result.stderr}
            
    except Exception as e:
        logger.error(f"Error running simulation: {str(e)}")
        return {'success': False, 'error': str(e)}

@app.route('/')
def index():
    """Serve the main page"""
    return send_from_directory('.', 'index.html')

@app.route('/<path:filename>')
def static_files(filename):
    """Serve static files"""
    return send_from_directory('.', filename)

@app.route('/api/upload', methods=['POST'])
def upload_files():
    """Handle file uploads"""
    try:
        if 'files' not in request.files:
            return jsonify({'success': False, 'error': 'No files provided'})
        
        files = request.files.getlist('files')
        uploaded_files = []
        
        for file in files:
            if file.filename == '':
                continue
                
            if file and allowed_file(file.filename):
                filename = secure_filename(file.filename)
                timestamp = datetime.now().strftime('%Y%m%d_%H%M%S_')
                filename = timestamp + filename
                
                filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
                file.save(filepath)
                
                file_info = {
                    'original_name': file.filename,
                    'filename': filename,
                    'filepath': filepath,
                    'size': os.path.getsize(filepath),
                    'type': get_file_type(file.filename)
                }
                
                # Process file based on type
                file_type = file_info['type']
                if file_type == 'ecg':
                    processing_result = process_ecg_data(filepath)
                elif file_type == 'mri':
                    processing_result = process_mri_data(filepath)
                elif file_type == 'dtm':
                    processing_result = process_dtm_data(filepath)
                else:
                    processing_result = {'success': True, 'metrics': {}}
                
                file_info['processing'] = processing_result
                uploaded_files.append(file_info)
                
                logger.info(f"Uploaded file: {filename}")
            else:
                return jsonify({'success': False, 'error': f'Invalid file type: {file.filename}'})
        
        return jsonify({
            'success': True,
            'files': uploaded_files,
            'message': f'Successfully uploaded {len(uploaded_files)} files'
        })
        
    except Exception as e:
        logger.error(f"Upload error: {str(e)}")
        return jsonify({'success': False, 'error': str(e)})

@app.route('/api/simulate', methods=['POST'])
def run_simulation():
    """Run simulation with uploaded files and parameters"""
    try:
        data = request.get_json()
        parameters = data.get('parameters', {})
        uploaded_files = data.get('files', [])
        
        logger.info(f"Running simulation with parameters: {parameters}")
        
        # Run C++ simulation
        simulation_results = run_cpp_simulation(parameters, uploaded_files)
        
        # Save results
        results_filename = f"simulation_results_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        results_path = os.path.join(app.config['RESULTS_FOLDER'], results_filename)
        
        with open(results_path, 'w') as f:
            json.dump(simulation_results, f, indent=2)
        
        simulation_results['results_file'] = results_filename
        
        return jsonify(simulation_results)
        
    except Exception as e:
        logger.error(f"Simulation error: {str(e)}")
        return jsonify({'success': False, 'error': str(e)})

@app.route('/api/results/<filename>')
def get_results(filename):
    """Get simulation results file"""
    try:
        return send_from_directory(app.config['RESULTS_FOLDER'], filename)
    except FileNotFoundError:
        return jsonify({'success': False, 'error': 'Results file not found'})

@app.route('/api/health')
def health_check():
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'timestamp': datetime.now().isoformat(),
        'version': '1.0.0'
    })

if __name__ == '__main__':
    print("MI Modeling Platform Web Server")
    print("===============================")
    print("Starting server on http://localhost:5000")
    print("Upload files and run simulations through the web interface")
    
    app.run(debug=True, host='0.0.0.0', port=5000)


