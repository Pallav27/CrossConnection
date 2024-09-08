#include <iostream>
#include <vector>
#include <cmath>
#include <Eigen/Dense>
#include <sndfile.h>

// Function to read audio file and return signal data
std::vector<double> readAudioFile(const std::string& filename) {
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return {};
    }

    std::vector<double> signal(sfinfo.frames);
    sf_readf_double(file, signal.data(), sfinfo.frames);
    sf_close(file);
    
    return signal;
}

// Function to normalize a signal (already normalized above, but keeping for future use)
std::vector<double> normalizeSignal(const std::vector<double>& signal) {
    double maxVal = *std::max_element(signal.begin(), signal.end());
    std::vector<double> normalizedSignal(signal.size());
    for (size_t i = 0; i < signal.size(); ++i) {
        normalizedSignal[i] = signal[i] / maxVal;
    }
    return normalizedSignal;
}

// Function to combine two signals (add them together)
std::vector<double> combineSignals(const std::vector<double>& signal1, const std::vector<double>& signal2) {
    int length = std::min(signal1.size(), signal2.size());
    std::vector<double> combinedSignal(length);
    for (int i = 0; i < length; i++) {
        combinedSignal[i] = signal1[i] + signal2[i];
    }
    return combinedSignal;
}

// Function to calculate the correlation between two signals using sliding window technique
std::vector<double> slidingWindowCorrelation(const std::vector<double>& signal, const std::vector<double>& interference, int windowSize) {
    int signalLength = signal.size();
    std::vector<double> correlation(signalLength - windowSize + 1, 0.0);

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

// Function to check if interference is present based on correlation threshold
bool detectInterference(const std::vector<double>& correlation, double threshold) {
    for (const auto& corr : correlation) {
        if (corr > threshold) {
            return true;  // Interference detected
        }
    }
    return false;  // No interference detected
}


int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <signal1_file> <signal2_file> <interference_file>" << std::endl;
        return 1;
    }

    // Read input files
    std::vector<double> signal1 = readAudioFile(argv[1]);
    std::vector<double> signal2 = readAudioFile(argv[2]);
    std::vector<double> interferenceSignal = readAudioFile(argv[3]);

    if (signal1.empty() || signal2.empty()) {
        std::cerr << "Error: One or more signals could not be loaded." << std::endl;
        return 1;
    }

    // Combine original signals and interference
    std::vector<double> combinedSignal = combineSignals(signal1, interferenceSignal);

    // Normalize signals
    combinedSignal = normalizeSignal(combinedSignal);
    interferenceSignal = normalizeSignal(interferenceSignal);

    // Define window size for sliding window correlation
    int windowSize = 100;  // Adjust based on signal characteristics

    // Calculate sliding window correlation
    std::vector<double> correlation = slidingWindowCorrelation(combinedSignal, interferenceSignal, windowSize);

    // Set a correlation threshold to detect interference
    double threshold = 0.8;  // Adjust this based on accuracy requirements

    // Detect interference
    if (detectInterference(correlation, threshold)) {
        std::cout << "Interference detected!" << std::endl;
    } else {
        std::cout << "No interference detected." << std::endl;
    }

    return 0;
}