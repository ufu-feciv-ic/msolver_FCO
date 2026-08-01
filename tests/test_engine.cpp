#include <iostream>
#include <cassert>
#include <cmath>
#include "engine/SimulationEngine.h"

void TestSimulationEngineExecution()
{
    std::cout << "[TEST] Running TestSimulationEngineExecution..." << std::endl;

    SimulationEngine engine;
    SimulationEngine::SimulationInput input;
    input.matrixSize = 5;
    input.loadFactor = 1.0;

    bool progressCalled = false;
    auto onProgress = [&progressCalled](float progress, const std::string& status) {
        progressCalled = true;
        assert(progress >= 0.0f && progress <= 1.0f);
        assert(!status.empty());
    };

    SimulationEngine::SimulationResult result = engine.RunSimulation(input, onProgress);

    assert(result.success == true);
    assert(result.solutionVector.size() == 5);
    assert(progressCalled == true);

    std::cout << "[TEST PASSED] TestSimulationEngineExecution" << std::endl;
}
