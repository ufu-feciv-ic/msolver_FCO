#include "engine/SimulationEngine.h"
#include <chrono>
#include <thread>

SimulationEngine::SimulationResult SimulationEngine::RunSimulation(const SimulationInput& input, ProgressCallback onProgress)
{
    const auto startTime = std::chrono::high_resolution_clock::now();

    if (onProgress) onProgress(0.1f, "Inicializando matrizes com Eigen...");

    const int n = input.matrixSize > 0 ? input.matrixSize : 10;
    
    // Criando matriz A e vetor b usando Eigen
    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(n, n);
    Eigen::VectorXd b = Eigen::VectorXd::Zero(n);

    for (int i = 0; i < n; ++i)
    {
        A(i, i) = 2.0 * input.loadFactor;
        if (i > 0) A(i, i - 1) = -1.0;
        if (i < n - 1) A(i, i + 1) = -1.0;
        b(i) = static_cast<double>(i + 1) * input.loadFactor;
    }

    if (onProgress) onProgress(0.5f, "Resolvendo sistema linear (Householder QR)...");
    
    // Simula processamento pesado se necessário
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Solver Eigen QR
    Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);

    if (onProgress) onProgress(0.9f, "Formatando resultados...");

    SimulationResult result;
    result.success = true;
    result.solutionVector.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        result.solutionVector.push_back(x(i));
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    result.message = "Simulação concluída com sucesso!";

    if (onProgress) onProgress(1.0f, "Pronto!");

    return result;
}
