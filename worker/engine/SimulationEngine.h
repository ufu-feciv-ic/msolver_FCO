#pragma once

#include <string>
#include <vector>
#include <functional>
#include <Eigen/Dense>

class SimulationEngine
{
public:
    struct SimulationInput
    {
        int matrixSize = 10;
        double loadFactor = 1.0;
    };

    struct SimulationResult
    {
        bool success = false;
        std::vector<double> solutionVector;
        double executionTimeMs = 0.0;
        std::string message;
    };

    using ProgressCallback = std::function<void(float progress, const std::string& status)>;

    SimulationResult RunSimulation(const SimulationInput& input, ProgressCallback onProgress);
};
