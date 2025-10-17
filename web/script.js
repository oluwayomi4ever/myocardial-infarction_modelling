// MI Modeling Platform JavaScript

class MIModelingPlatform {
    constructor() {
        this.uploadedFiles = [];
        this.simulationResults = null;
        this.isProcessing = false;
        this.initializeEventListeners();
    }

    initializeEventListeners() {
        // File upload handling
        const uploadArea = document.getElementById('uploadArea');
        const fileInput = document.getElementById('fileInput');

        // Drag and drop events
        uploadArea.addEventListener('dragover', this.handleDragOver.bind(this));
        uploadArea.addEventListener('dragleave', this.handleDragLeave.bind(this));
        uploadArea.addEventListener('drop', this.handleDrop.bind(this));
        uploadArea.addEventListener('click', () => fileInput.click());

        // File input change
        fileInput.addEventListener('change', this.handleFileSelect.bind(this));

        // Form submission
        document.getElementById('parameterForm').addEventListener('submit', this.handleFormSubmit.bind(this));

        // Navigation
        document.querySelectorAll('.nav-link').forEach(link => {
            link.addEventListener('click', this.handleNavigation.bind(this));
        });
    }

    handleDragOver(e) {
        e.preventDefault();
        e.currentTarget.classList.add('dragover');
    }

    handleDragLeave(e) {
        e.preventDefault();
        e.currentTarget.classList.remove('dragover');
    }

    handleDrop(e) {
        e.preventDefault();
        e.currentTarget.classList.remove('dragover');
        const files = Array.from(e.dataTransfer.files);
        this.processFiles(files);
    }

    handleFileSelect(e) {
        const files = Array.from(e.target.files);
        this.processFiles(files);
    }

    processFiles(files) {
        files.forEach(file => {
            if (this.validateFile(file)) {
                this.uploadedFiles.push(file);
                this.displayFile(file);
            } else {
                this.showNotification(`Invalid file: ${file.name}`, 'error');
            }
        });

        if (this.uploadedFiles.length > 0) {
            document.getElementById('uploadedFiles').style.display = 'block';
        }
    }

    validateFile(file) {
        const allowedTypes = [
            'text/csv',
            'text/plain',
            'application/octet-stream',
            'application/dicom',
            'application/x-hdf'
        ];
        
        const allowedExtensions = ['.csv', '.dat', '.txt', '.h5', '.hdf5', '.dcm', '.dicom'];
        const fileExtension = file.name.toLowerCase().substring(file.name.lastIndexOf('.'));
        
        return allowedTypes.includes(file.type) || allowedExtensions.includes(fileExtension);
    }

    displayFile(file) {
        const fileList = document.getElementById('fileList');
        const fileItem = document.createElement('div');
        fileItem.className = 'file-item fade-in';
        
        const extension = file.name.split('.').pop().toLowerCase();
        const iconClass = this.getFileIcon(extension);
        
        fileItem.innerHTML = `
            <div class="file-info">
                <i class="fas ${iconClass} file-icon ${extension}"></i>
                <div class="file-details">
                    <h6>${file.name}</h6>
                    <small>${this.formatFileSize(file.size)} • ${file.type || 'Unknown type'}</small>
                </div>
            </div>
            <div class="file-actions">
                <button class="btn btn-sm btn-outline-primary" onclick="mimodeling.previewFile('${file.name}')">
                    <i class="fas fa-eye"></i>
                </button>
                <button class="btn btn-sm btn-outline-danger" onclick="mimodeling.removeFile('${file.name}')">
                    <i class="fas fa-trash"></i>
                </button>
            </div>
        `;
        
        fileList.appendChild(fileItem);
    }

    getFileIcon(extension) {
        const iconMap = {
            'csv': 'fa-file-csv',
            'dat': 'fa-file-alt',
            'txt': 'fa-file-alt',
            'h5': 'fa-file-code',
            'hdf5': 'fa-file-code',
            'dcm': 'fa-file-medical',
            'dicom': 'fa-file-medical'
        };
        return iconMap[extension] || 'fa-file';
    }

    formatFileSize(bytes) {
        if (bytes === 0) return '0 Bytes';
        const k = 1024;
        const sizes = ['Bytes', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }

    removeFile(fileName) {
        this.uploadedFiles = this.uploadedFiles.filter(file => file.name !== fileName);
        
        // Remove from DOM
        const fileItems = document.querySelectorAll('.file-item');
        fileItems.forEach(item => {
            if (item.querySelector('h6').textContent === fileName) {
                item.remove();
            }
        });

        if (this.uploadedFiles.length === 0) {
            document.getElementById('uploadedFiles').style.display = 'none';
        }
    }

    previewFile(fileName) {
        const file = this.uploadedFiles.find(f => f.name === fileName);
        if (!file) return;

        const reader = new FileReader();
        reader.onload = (e) => {
            const content = e.target.result;
            this.showFilePreview(fileName, content);
        };
        reader.readAsText(file);
    }

    showFilePreview(fileName, content) {
        const modal = new bootstrap.Modal(document.createElement('div'));
        const modalHtml = `
            <div class="modal fade" tabindex="-1">
                <div class="modal-dialog modal-lg">
                    <div class="modal-content">
                        <div class="modal-header">
                            <h5 class="modal-title">File Preview: ${fileName}</h5>
                            <button type="button" class="btn-close" data-bs-dismiss="modal"></button>
                        </div>
                        <div class="modal-body">
                            <pre style="max-height: 400px; overflow-y: auto; background-color: #f8f9fa; padding: 1rem; border-radius: 5px;">${content.substring(0, 2000)}${content.length > 2000 ? '\n... (truncated)' : ''}</pre>
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">Close</button>
                        </div>
                    </div>
                </div>
            </div>
        `;
        
        document.body.insertAdjacentHTML('beforeend', modalHtml);
        const newModal = document.querySelector('.modal:last-child');
        const bsModal = new bootstrap.Modal(newModal);
        bsModal.show();
        
        newModal.addEventListener('hidden.bs.modal', () => {
            newModal.remove();
        });
    }

    handleFormSubmit(e) {
        e.preventDefault();
        
        if (this.uploadedFiles.length === 0) {
            this.showNotification('Please upload at least one file before running simulation', 'warning');
            return;
        }

        if (this.isProcessing) {
            this.showNotification('Simulation is already running', 'warning');
            return;
        }

        this.runSimulation();
    }

    async runSimulation() {
        this.isProcessing = true;
        this.showProcessingCard();
        
        try {
            // Get form parameters
            const parameters = this.getFormParameters();
            
            // Simulate processing steps
            await this.simulateProcessing();
            
            // Generate mock results
            this.simulationResults = this.generateMockResults(parameters);
            
            // Display results
            this.displayResults(this.simulationResults);
            
            this.showNotification('Simulation completed successfully!', 'success');
            
        } catch (error) {
            this.showNotification('Simulation failed: ' + error.message, 'error');
        } finally {
            this.isProcessing = false;
            this.hideProcessingCard();
        }
    }

    getFormParameters() {
        return {
            modelType: document.getElementById('modelType').value,
            gridWidth: parseInt(document.getElementById('gridWidth').value),
            gridHeight: parseInt(document.getElementById('gridHeight').value),
            timeStep: parseFloat(document.getElementById('timeStep').value),
            simulationSteps: parseInt(document.getElementById('simulationSteps').value),
            includeMI: document.getElementById('includeMI').checked
        };
    }

    async simulateProcessing() {
        const steps = [
            'Reading uploaded files...',
            'Validating data format...',
            'Initializing model parameters...',
            'Setting up computational grid...',
            'Running simulation...',
            'Processing results...',
            'Generating visualizations...'
        ];

        for (let i = 0; i < steps.length; i++) {
            await this.updateProcessingStep(steps[i], (i + 1) / steps.length);
            await this.sleep(1000);
        }
    }

    updateProcessingStep(message, progress) {
        document.getElementById('processingStatus').textContent = message;
        document.querySelector('.progress-bar').style.width = (progress * 100) + '%';
    }

    showProcessingCard() {
        document.getElementById('processingCard').style.display = 'block';
    }

    hideProcessingCard() {
        document.getElementById('processingCard').style.display = 'none';
    }

    generateMockResults(parameters) {
        // Generate mock simulation results
        const results = {
            parameters: parameters,
            metrics: {
                actionPotentialDuration: 280 + Math.random() * 20,
                conductionVelocity: 0.45 + Math.random() * 0.1,
                ejectionFraction: 55 + Math.random() * 10,
                wallMotionScore: 1.2 + Math.random() * 0.5
            },
            timeSeries: this.generateTimeSeries(),
            spatialData: this.generateSpatialData(parameters.gridWidth, parameters.gridHeight)
        };

        return results;
    }

    generateTimeSeries() {
        const data = [];
        for (let i = 0; i < 1000; i++) {
            data.push({
                time: i * 0.01,
                membranePotential: -85 + 100 * Math.sin(i * 0.1) * Math.exp(-i * 0.001),
                recoveryVariable: 0.5 + 0.3 * Math.sin(i * 0.05)
            });
        }
        return data;
    }

    generateSpatialData(width, height) {
        const data = [];
        for (let y = 0; y < height; y += 5) {
            for (let x = 0; x < width; x += 5) {
                data.push({
                    x: x,
                    y: y,
                    value: Math.sin(x * 0.1) * Math.cos(y * 0.1) * 50
                });
            }
        }
        return data;
    }

    displayResults(results) {
        const resultsContent = document.getElementById('resultsContent');
        
        resultsContent.innerHTML = `
            <div class="results-grid">
                <div class="result-card">
                    <div class="result-title">Action Potential Duration</div>
                    <div class="result-value">${results.metrics.actionPotentialDuration.toFixed(1)}<span class="result-unit">ms</span></div>
                </div>
                <div class="result-card">
                    <div class="result-title">Conduction Velocity</div>
                    <div class="result-value">${results.metrics.conductionVelocity.toFixed(2)}<span class="result-unit">m/s</span></div>
                </div>
                <div class="result-card">
                    <div class="result-title">Ejection Fraction</div>
                    <div class="result-value">${results.metrics.ejectionFraction.toFixed(1)}<span class="result-unit">%</span></div>
                </div>
                <div class="result-card">
                    <div class="result-title">Wall Motion Score</div>
                    <div class="result-value">${results.metrics.wallMotionScore.toFixed(2)}</div>
                </div>
            </div>
            
            <div class="chart-container">
                <canvas id="timeSeriesChart"></canvas>
            </div>
            
            <div class="mt-3">
                <button class="btn btn-primary" onclick="mimodeling.showDetailedResults()">
                    <i class="fas fa-chart-bar me-2"></i>View Detailed Results
                </button>
                <button class="btn btn-outline-primary" onclick="mimodeling.exportResults()">
                    <i class="fas fa-download me-2"></i>Export Data
                </button>
            </div>
        `;

        // Create time series chart
        this.createTimeSeriesChart(results.timeSeries);
    }

    createTimeSeriesChart(timeSeries) {
        const ctx = document.getElementById('timeSeriesChart').getContext('2d');
        
        new Chart(ctx, {
            type: 'line',
            data: {
                labels: timeSeries.map(d => d.time.toFixed(2)),
                datasets: [{
                    label: 'Membrane Potential (mV)',
                    data: timeSeries.map(d => d.membranePotential),
                    borderColor: '#3498db',
                    backgroundColor: 'rgba(52, 152, 219, 0.1)',
                    tension: 0.1
                }, {
                    label: 'Recovery Variable',
                    data: timeSeries.map(d => d.recoveryVariable),
                    borderColor: '#e74c3c',
                    backgroundColor: 'rgba(231, 76, 60, 0.1)',
                    tension: 0.1,
                    yAxisID: 'y1'
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        type: 'linear',
                        display: true,
                        position: 'left',
                        title: {
                            display: true,
                            text: 'Membrane Potential (mV)'
                        }
                    },
                    y1: {
                        type: 'linear',
                        display: true,
                        position: 'right',
                        title: {
                            display: true,
                            text: 'Recovery Variable'
                        },
                        grid: {
                            drawOnChartArea: false,
                        },
                    }
                },
                plugins: {
                    title: {
                        display: true,
                        text: 'Cardiac Electrophysiology Simulation Results'
                    },
                    legend: {
                        display: true,
                        position: 'top'
                    }
                }
            }
        });
    }

    showDetailedResults() {
        const modal = document.getElementById('resultsModal');
        const modalResults = document.getElementById('modalResults');
        
        modalResults.innerHTML = `
            <div class="row">
                <div class="col-md-6">
                    <h6>Simulation Parameters</h6>
                    <ul class="list-group list-group-flush">
                        <li class="list-group-item d-flex justify-content-between">
                            <span>Model Type:</span>
                            <span>${this.simulationResults.parameters.modelType}</span>
                        </li>
                        <li class="list-group-item d-flex justify-content-between">
                            <span>Grid Size:</span>
                            <span>${this.simulationResults.parameters.gridWidth} × ${this.simulationResults.parameters.gridHeight}</span>
                        </li>
                        <li class="list-group-item d-flex justify-content-between">
                            <span>Time Step:</span>
                            <span>${this.simulationResults.parameters.timeStep} ms</span>
                        </li>
                        <li class="list-group-item d-flex justify-content-between">
                            <span>Simulation Steps:</span>
                            <span>${this.simulationResults.parameters.simulationSteps}</span>
                        </li>
                    </ul>
                </div>
                <div class="col-md-6">
                    <h6>Uploaded Files</h6>
                    <ul class="list-group list-group-flush">
                        ${this.uploadedFiles.map(file => `
                            <li class="list-group-item d-flex justify-content-between">
                                <span>${file.name}</span>
                                <span class="badge bg-primary">${this.formatFileSize(file.size)}</span>
                            </li>
                        `).join('')}
                    </ul>
                </div>
            </div>
        `;
        
        const bsModal = new bootstrap.Modal(modal);
        bsModal.show();
    }

    exportResults() {
        if (!this.simulationResults) {
            this.showNotification('No results to export', 'warning');
            return;
        }

        const exportData = {
            parameters: this.simulationResults.parameters,
            metrics: this.simulationResults.metrics,
            timestamp: new Date().toISOString(),
            files: this.uploadedFiles.map(f => ({ name: f.name, size: f.size }))
        };

        const blob = new Blob([JSON.stringify(exportData, null, 2)], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `mi_simulation_results_${Date.now()}.json`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);

        this.showNotification('Results exported successfully!', 'success');
    }

    showNotification(message, type = 'info') {
        // Create notification element
        const notification = document.createElement('div');
        notification.className = `alert alert-${type === 'error' ? 'danger' : type} alert-dismissible fade show position-fixed`;
        notification.style.cssText = 'top: 20px; right: 20px; z-index: 9999; min-width: 300px;';
        
        notification.innerHTML = `
            ${message}
            <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
        `;
        
        document.body.appendChild(notification);
        
        // Auto-remove after 5 seconds
        setTimeout(() => {
            if (notification.parentNode) {
                notification.remove();
            }
        }, 5000);
    }

    handleNavigation(e) {
        e.preventDefault();
        const target = e.target.getAttribute('href').substring(1);
        
        // Update active nav item
        document.querySelectorAll('.nav-link').forEach(link => link.classList.remove('active'));
        e.target.classList.add('active');
        
        // Scroll to section (if it exists)
        const element = document.getElementById(target);
        if (element) {
            element.scrollIntoView({ behavior: 'smooth' });
        }
    }

    sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
}

// Initialize the platform when the page loads
let mimodeling;
document.addEventListener('DOMContentLoaded', () => {
    mimodeling = new MIModelingPlatform();
});

// Global functions for onclick handlers
window.mimodeling = null;
document.addEventListener('DOMContentLoaded', () => {
    window.mimodeling = mimodeling;
});


