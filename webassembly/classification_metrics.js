// Classification Metrics Calculator for MI Modeling
// This provides F1 score, recall, precision, accuracy, and confusion matrix

class ClassificationMetrics {
    constructor() {
        this.results = [];
    }
    
    // Generate synthetic ground truth for MI regions (central area)
    generateGroundTruth(gridWidth, gridHeight, miRadius = 0.2) {
        const groundTruth = [];
        const centerX = gridWidth / 2;
        const centerY = gridHeight / 2;
        const radius = Math.min(gridWidth, gridHeight) * miRadius;
        
        for (let y = 0; y < gridHeight; y++) {
            groundTruth[y] = [];
            for (let x = 0; x < gridWidth; x++) {
                const distance = Math.sqrt((x - centerX) ** 2 + (y - centerY) ** 2);
                groundTruth[y][x] = distance < radius;
            }
        }
        
        return groundTruth;
    }
    
    // Classify tissue based on membrane potential threshold
    classifyTissue(membranePotential, threshold = -0.5) {
        const predictions = [];
        
        for (let y = 0; y < membranePotential.length; y++) {
            predictions[y] = [];
            for (let x = 0; x < membranePotential[y].length; x++) {
                // Classify as MI if membrane potential is outside normal range
                // Normal range is typically -80mV to -60mV, but our model uses normalized values
                predictions[y][x] = membranePotential[y][x] < threshold || membranePotential[y][x] > 0.5;
            }
        }
        
        return predictions;
    }
    
    // Calculate confusion matrix
    calculateConfusionMatrix(groundTruth, predictions) {
        let tp = 0, fp = 0, tn = 0, fn = 0;
        
        console.log('Confusion Matrix Debug:', {
            groundTruthLength: groundTruth.length,
            predictionsLength: predictions.length,
            sampleGroundTruth: groundTruth[0] ? groundTruth[0].slice(0, 3) : 'N/A',
            samplePredictions: predictions[0] ? predictions[0].slice(0, 3) : 'N/A'
        });
        
        for (let y = 0; y < groundTruth.length; y++) {
            for (let x = 0; x < groundTruth[y].length; x++) {
                const actual = groundTruth[y][x];
                const predicted = predictions[y][x];
                
                if (actual && predicted) tp++;
                else if (!actual && predicted) fp++;
                else if (!actual && !predicted) tn++;
                else if (actual && !predicted) fn++;
            }
        }
        
        console.log('Raw Confusion Matrix:', { tp, fp, tn, fn, total: tp + fp + tn + fn });
        
        return { tp, fp, tn, fn };
    }
    
    // Calculate all classification metrics
    calculateMetrics(groundTruth, predictions) {
        const { tp, fp, tn, fn } = this.calculateConfusionMatrix(groundTruth, predictions);
        
        const total = tp + fp + tn + fn;
        const accuracy = total > 0 ? (tp + tn) / total : 0;
        const precision = (tp + fp) > 0 ? tp / (tp + fp) : 0;
        const recall = (tp + fn) > 0 ? tp / (tp + fn) : 0;
        const specificity = (tn + fp) > 0 ? tn / (tn + fp) : 0;
        const f1Score = (precision + recall) > 0 ? 2 * (precision * recall) / (precision + recall) : 0;
        
        console.log('Metrics Calculation Debug:', {
            rawValues: { tp, fp, tn, fn, total },
            rawCalculations: { accuracy, precision, recall, specificity, f1Score },
            afterPercentage: {
                accuracy: Math.round(accuracy * 100 * 100) / 100,
                precision: Math.round(precision * 100 * 100) / 100,
                recall: Math.round(recall * 100 * 100) / 100,
                f1Score: Math.round(f1Score * 100 * 100) / 100
            }
        });
        
        return {
            accuracy: Math.round(accuracy * 100 * 100) / 100, // Round to 2 decimal places
            precision: Math.round(precision * 100 * 100) / 100,
            recall: Math.round(recall * 100 * 100) / 100,
            specificity: Math.round(specificity * 100 * 100) / 100,
            f1Score: Math.round(f1Score * 100 * 100) / 100,
            truePositives: tp,
            falsePositives: fp,
            trueNegatives: tn,
            falseNegatives: fn,
            confusionMatrix: [[tp, fp], [fn, tn]]
        };
    }
    
    // Analyze membrane potential data and return classification metrics
    analyzeMembranePotential(membranePotential) {
        // Handle both 1D and 2D arrays
        let gridWidth, gridHeight, membraneData;
        
        if (Array.isArray(membranePotential) && Array.isArray(membranePotential[0])) {
            // 2D array format
            gridWidth = membranePotential[0].length;
            gridHeight = membranePotential.length;
            membraneData = membranePotential;
        } else {
            // 1D array format - assume square grid
            const totalCells = membranePotential.length;
            gridWidth = gridHeight = Math.sqrt(totalCells);
            
            // Convert 1D to 2D
            membraneData = [];
            for (let y = 0; y < gridHeight; y++) {
                membraneData[y] = [];
                for (let x = 0; x < gridWidth; x++) {
                    membraneData[y][x] = membranePotential[y * gridWidth + x];
                }
            }
        }
        
        const groundTruth = this.generateGroundTruth(gridWidth, gridHeight);
        const predictions = this.classifyTissue(membraneData);
        const metrics = this.calculateMetrics(groundTruth, predictions);
        
        // Debug logging
        console.log('Classification Debug:', {
            gridSize: `${gridWidth}x${gridHeight}`,
            membraneSample: membraneData[0] ? membraneData[0].slice(0, 5) : 'N/A',
            groundTruthSample: groundTruth[0] ? groundTruth[0].slice(0, 5) : 'N/A',
            predictionsSample: predictions[0] ? predictions[0].slice(0, 5) : 'N/A',
            rawMetrics: {
                tp: metrics.truePositives,
                fp: metrics.falsePositives,
                tn: metrics.trueNegatives,
                fn: metrics.falseNegatives
            },
            calculatedMetrics: {
                accuracy: metrics.accuracy,
                precision: metrics.precision,
                recall: metrics.recall,
                f1Score: metrics.f1Score
            }
        });
        
        return metrics;
    }
    
    // Generate epoch-based metrics for training
    generateEpochMetrics(membranePotentialHistory) {
        const epochMetrics = [];
        
        for (let i = 0; i < membranePotentialHistory.length; i++) {
            const metrics = this.analyzeMembranePotential(membranePotentialHistory[i]);
            epochMetrics.push({
                epoch: i,
                accuracy: metrics.accuracy,
                precision: metrics.precision,
                recall: metrics.recall,
                f1Score: metrics.f1Score,
                specificity: metrics.specificity
            });
        }
        
        return epochMetrics;
    }
}

// Export for use in the main application
window.ClassificationMetrics = ClassificationMetrics;
