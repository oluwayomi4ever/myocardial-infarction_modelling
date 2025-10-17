// Fixed Classification Metrics Calculator for MI Modeling
// This provides F1 score, recall, precision, accuracy, and confusion matrix

class ClassificationMetricsFixed {
    constructor() {
        this.results = [];
    }
    
    // Generate synthetic ground truth for MI regions (central area)
    generateGroundTruth(gridWidth, gridHeight, miRadius = 0.2) {
        const groundTruth = [];
        const centerX = gridWidth / 2;
        const centerY = gridHeight / 2;
        const radius = Math.min(gridWidth, gridHeight) * miRadius;
        
        console.log(`Generating ground truth: ${gridWidth}x${gridHeight}, center: (${centerX}, ${centerY}), radius: ${radius}`);
        
        for (let y = 0; y < gridHeight; y++) {
            groundTruth[y] = [];
            for (let x = 0; x < gridWidth; x++) {
                const distance = Math.sqrt((x - centerX) ** 2 + (y - centerY) ** 2);
                groundTruth[y][x] = distance < radius; // true for MI, false for healthy
            }
        }
        
        return groundTruth;
    }
    
    // Classify tissue based on membrane potential threshold
    classifyTissue(membranePotential) {
        const predictions = [];
        const threshold = 0.1; // Simple threshold for demonstration
        
        console.log(`Classifying tissue with threshold: ${threshold}`);
        
        for (let y = 0; y < membranePotential.length; y++) {
            predictions[y] = [];
            for (let x = 0; x < membranePotential[y].length; x++) {
                // Simple classification: if membrane potential is above threshold, classify as MI
                predictions[y][x] = Math.abs(membranePotential[y][x]) > threshold;
            }
        }
        
        return predictions;
    }
    
    // Calculate confusion matrix
    calculateConfusionMatrix(groundTruth, predictions) {
        let tp = 0, fp = 0, tn = 0, fn = 0;
        
        console.log(`Calculating confusion matrix for ${groundTruth.length}x${groundTruth[0].length} grid`);
        
        for (let y = 0; y < groundTruth.length; y++) {
            for (let x = 0; x < groundTruth[y].length; x++) {
                const actual = groundTruth[y][x];
                const predicted = predictions[y][x];
                
                if (actual && predicted) tp++;      // True Positive: MI detected correctly
                else if (!actual && predicted) fp++; // False Positive: Healthy classified as MI
                else if (!actual && !predicted) tn++; // True Negative: Healthy classified correctly
                else if (actual && !predicted) fn++;  // False Negative: MI missed
            }
        }
        
        const total = tp + fp + tn + fn;
        console.log(`Confusion Matrix: TP=${tp}, FP=${fp}, TN=${tn}, FN=${fn}, Total=${total}`);
        
        return { tp, fp, tn, fn };
    }
    
    // Calculate all classification metrics
    calculateMetrics(groundTruth, predictions) {
        const { tp, fp, tn, fn } = this.calculateConfusionMatrix(groundTruth, predictions);
        
        const total = tp + fp + tn + fn;
        
        // Calculate raw metrics (0-1 range)
        const accuracy = total > 0 ? (tp + tn) / total : 0;
        const precision = (tp + fp) > 0 ? tp / (tp + fp) : 0;
        const recall = (tp + fn) > 0 ? tp / (tp + fn) : 0;
        const specificity = (tn + fp) > 0 ? tn / (tn + fp) : 0;
        const f1Score = (precision + recall) > 0 ? 2 * (precision * recall) / (precision + recall) : 0;
        
        console.log(`Raw calculations: accuracy=${accuracy}, precision=${precision}, recall=${recall}, f1=${f1Score}`);
        
        // Convert to percentages and round to 2 decimal places
        const result = {
            accuracy: Math.round(accuracy * 10000) / 100,    // Convert to percentage
            precision: Math.round(precision * 10000) / 100,
            recall: Math.round(recall * 10000) / 100,
            specificity: Math.round(specificity * 10000) / 100,
            sensitivity: Math.round(recall * 10000) / 100,   // Sensitivity = Recall in medical terms
            f1Score: Math.round(f1Score * 10000) / 100,
            truePositives: tp,
            falsePositives: fp,
            trueNegatives: tn,
            falseNegatives: fn,
            confusionMatrix: [[tp, fp], [fn, tn]]
        };
        
        console.log(`Final metrics:`, result);
        
        return result;
    }
    
    // Analyze membrane potential data and return classification metrics
    analyzeMembranePotential(membranePotential) {
        console.log('=== Starting Classification Analysis ===');
        console.log('Input data type:', Array.isArray(membranePotential) ? 'Array' : typeof membranePotential);
        console.log('Input length:', membranePotential.length);
        
        // Handle both 1D and 2D arrays
        let gridWidth, gridHeight, membraneData;
        
        if (Array.isArray(membranePotential) && Array.isArray(membranePotential[0])) {
            // 2D array format
            gridWidth = membranePotential[0].length;
            gridHeight = membranePotential.length;
            membraneData = membranePotential;
            console.log(`Detected 2D array: ${gridWidth}x${gridHeight}`);
        } else {
            // 1D array format - assume square grid
            const totalCells = membranePotential.length;
            gridWidth = gridHeight = Math.sqrt(totalCells);
            
            if (!Number.isInteger(gridWidth)) {
                console.error('Cannot create square grid from 1D array of length:', totalCells);
                return {
                    accuracy: 0, precision: 0, recall: 0, specificity: 0, f1Score: 0,
                    truePositives: 0, falsePositives: 0, trueNegatives: 0, falseNegatives: 0,
                    confusionMatrix: [[0, 0], [0, 0]]
                };
            }
            
            // Convert 1D to 2D
            membraneData = [];
            for (let y = 0; y < gridHeight; y++) {
                membraneData[y] = [];
                for (let x = 0; x < gridWidth; x++) {
                    membraneData[y][x] = membranePotential[y * gridWidth + x];
                }
            }
            console.log(`Converted 1D to 2D: ${gridWidth}x${gridHeight}`);
        }
        
        // Sample some values for debugging
        console.log('Sample membrane values:', membraneData[0] ? membraneData[0].slice(0, 5) : 'N/A');
        
        const groundTruth = this.generateGroundTruth(gridWidth, gridHeight);
        const predictions = this.classifyTissue(membraneData);
        const metrics = this.calculateMetrics(groundTruth, predictions);
        
        console.log('=== Classification Analysis Complete ===');
        
        return metrics;
    }
    
    // Generate epoch-based metrics for training
    generateEpochMetrics(membranePotentialHistory) {
        const epochMetrics = [];
        
        for (let i = 0; i < membranePotentialHistory.length; i++) {
            console.log(`Processing epoch ${i + 1}/${membranePotentialHistory.length}`);
            const metrics = this.analyzeMembranePotential(membranePotentialHistory[i]);
            epochMetrics.push({
                epoch: i + 1,
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
window.ClassificationMetricsFixed = ClassificationMetricsFixed;
