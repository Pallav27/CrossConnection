#include <iostream>
#include <vector>
#include <cmath>
#include <Eigen/Dense>
#include <sndfile.h>

using namespace std;

// Function to read audio file and return signal data
vector<double> readAudioFile(const string& filename) {
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        cerr << "Error: Could not open file " << filename << endl;
        return {};
    }

    vector<double> signal(sfinfo.frames);
    sf_readf_double(file, signal.data(), sfinfo.frames);
    sf_close(file);
    
    return signal;
}

// Function to normalize a signal
vector<double> normalizeSignal(const vector<double>& signal) {
    double maxVal = *max_element(signal.begin(), signal.end());
    vector<double> normalizedSignal(signal.size());
    for (size_t i = 0; i < signal.size(); ++i) {
        normalizedSignal[i] = signal[i] / maxVal;
    }
    return normalizedSignal;
}

// Function to combine two signals (add them together)
vector<double> combineSignals(const vector<double>& signal1, const vector<double>& signal2) {
    int length = min(signal1.size(), signal2.size());
    vector<double> combinedSignal(length);
    for (int i = 0; i < length; i++) {
        combinedSignal[i] = signal1[i] + signal2[i];
    }
    return combinedSignal;
}

// Function to calculate the correlation between two signals using sliding window technique
vector<double> slidingWindowCorrelation(const vector<double>& signal, const vector<double>& interference, int windowSize) {
    int signalLength = signal.size();
    vector<double> correlation(signalLength - windowSize + 1, 0.0);

    for (int i = 0; i <= signalLength - windowSize; i++) {
        Eigen::VectorXd signalWindow = Eigen::Map<const Eigen::VectorXd>(signal.data() + i, windowSize);
        Eigen::VectorXd interferenceWindow = Eigen::Map<const Eigen::VectorXd>(interference.data(), windowSize);

        double dotProduct = signalWindow.dot(interferenceWindow);
        double magnitudeSignal = signalWindow.norm();
        double magnitudeInterference = interferenceWindow.norm();

        correlation[i] = dotProduct / (magnitudeSignal * magnitudeInterference);  // Cosine similarity
    }

    return correlation;
}

// Function to calculate a moving average of correlation values
vector<double> calculateMovingAverage(const vector<double>& correlation, int windowSize) {
    vector<double> movingAverage;
    double sum = 0.0;

    for (int i = 0; i < correlation.size(); ++i) {
        sum += correlation[i];
        if (i >= windowSize - 1) {
            movingAverage.push_back(sum / windowSize);
            sum -= correlation[i - windowSize + 1];
        }
    }

    return movingAverage;
}

// Function to check if interference is present based on consecutive threshold crossings
bool detectInterference(const vector<double>& correlation, double threshold, int minConsecutive) {
    int count = 0;  // Count consecutive above-threshold values

    for (const auto& corr : correlation) {
        if (corr > threshold) {
            count++;
            if (count >= minConsecutive) {
                return true;  // Interference detected
            }
        } else {
            count = 0;  // Reset count if the threshold is not exceeded
        }
    }
    return false;  // No interference detected
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <signal1_file> <signal2_file> <interference_file>" << endl;
        return 1;
    }

    //input files
    vector<double> signal1 = readAudioFile(argv[1]);
    vector<double> signal2 = readAudioFile(argv[2]);
    vector<double> interferenceSignal = readAudioFile(argv[3]);

    if (signal1.empty() || signal2.empty() || interferenceSignal.empty()) {
        cerr << "Error: One or more signals could not be loaded." << endl;
        return 1;
    }

    // Combine original signals and interference
    vector<double> combinedSignal = combineSignals(signal1, interferenceSignal);

    // Normalize signals
    combinedSignal = normalizeSignal(combinedSignal);
    interferenceSignal = normalizeSignal(interferenceSignal);

    // Define window size for sliding window correlation
    int windowSize = 100;  // Adjust based on signal characteristics

    // Calculate sliding window correlation
    vector<double> correlation = slidingWindowCorrelation(combinedSignal, interferenceSignal, windowSize);

    // Smooth correlation using moving average
    int avgWindowSize = 10;  // Adjust for smoothing effect
    vector<double> smoothedCorrelation = calculateMovingAverage(correlation, avgWindowSize);

    // Set parameters for interference detection
    double threshold = 0.75;  // Adjust this based on accuracy requirements
    int minConsecutive = 3;   // Minimum consecutive windows above threshold

    // Detect interference
    if (detectInterference(smoothedCorrelation, threshold, minConsecutive)) {
        cout << "Interference detected!" << endl;
    } else {
        cout << "No interference detected." << endl;
    }

    return 0;
}
