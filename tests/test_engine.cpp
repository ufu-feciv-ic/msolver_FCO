#include <iostream>
#include <cassert>
#include <cmath>
#include "engine/SimulationEngine.h"

void TestSimulationEngineExecution()
{
    std::cout << "[TEST] Running TestSimulationEngineExecution (Concrete Cross Section)..." << std::endl;

    SimulationEngine engine;
    SimulationEngine::SimulationInput input;
    
    // Seção retangular 20x50 cm
    input.polygonVertices = {
        Point(-10.0, -25.0),
        Point( 10.0, -25.0),
        Point( 10.0,  25.0),
        Point(-10.0,  25.0)
    };

    // 4 barras Φ 16mm no fundo, 2 barras Φ 12.5mm no topo
    input.rebars = {
        { -7.0, -22.0, 16.0 },
        { -2.33, -22.0, 16.0 },
        {  2.33, -22.0, 16.0 },
        {  7.0, -22.0, 16.0 },
        { -7.0,  22.0, 12.5 },
        {  7.0,  22.0, 12.5 }
    };

    input.fck = 30.0;
    input.gammaC = 1.4;
    input.concreteModelType = 1;

    input.fyk = 500.0;
    input.gammaS = 1.15;
    input.Es = 210.0;

    input.Nsd = 0.0;
    input.Msdx = 20.0;
    input.Msdy = 5.0;

    bool progressCalled = false;
    auto onProgress = [&progressCalled](float progress, const std::string& status) {
        progressCalled = true;
        assert(progress >= 0.0f && progress <= 1.0f);
        assert(!status.empty());
    };

    SimulationEngine::SimulationResult result = engine.RunSimulation(input, onProgress);

    assert(result.success == true);
    assert(!result.envelopeMoments.empty());
    assert(result.area > 0.0);
    assert(progressCalled == true);

    std::cout << "[TEST PASSED] TestSimulationEngineExecution - Envoltória gerada com " 
              << result.envelopeMoments.size() << " pontos." << std::endl;
}

void TestSteelSizingExecution()
{
    std::cout << "[TEST] Running TestSteelSizingExecution (Bisection Sizing Solver)..." << std::endl;

    SimulationEngine engine;
    SimulationEngine::SimulationInput input;

    // Seção retangular 20x50 cm (Ac = 1000 cm²)
    input.polygonVertices = {
        Point(-10.0, -25.0),
        Point( 10.0, -25.0),
        Point( 10.0,  25.0),
        Point(-10.0,  25.0)
    };

    // Armadura de referência inicial (4 barras 16mm no fundo, 2 barras 12.5mm no topo)
    input.rebars = {
        { -7.0, -22.0, 16.0 },
        { -2.33, -22.0, 16.0 },
        {  2.33, -22.0, 16.0 },
        {  7.0, -22.0, 16.0 },
        { -7.0,  22.0, 12.5 },
        {  7.0,  22.0, 12.5 }
    };

    input.fck = 30.0;
    input.gammaC = 1.4;
    input.concreteModelType = 1;

    input.fyk = 500.0;
    input.gammaS = 1.15;
    input.Es = 210.0;

    // Carga de teste (Flexão Composta Oblíqua: Nsd = -200 kN, Msdx = 60 kN.m, Msdy = 15 kN.m)
    input.Nsd = -200.0;
    input.Msdx = 60.0;
    input.Msdy = 15.0;

    SimulationEngine::SizingResult sizingRes = engine.RunSizingSimulation(input, nullptr);

    assert(sizingRes.success == true);
    assert(sizingRes.requiredSteelArea > 0.0);
    assert(sizingRes.omegaMechanicalRatio > 0.0);
    assert(sizingRes.iterationsCount > 0);
    assert(sizingRes.verificationResult.isSafe == true);

    std::cout << "[TEST PASSED] TestSteelSizingExecution - As_req = " 
              << sizingRes.requiredSteelArea << " cm^2, Taxa omega = " 
              << sizingRes.omegaMechanicalRatio << " em " 
              << sizingRes.iterationsCount << " iterações." << std::endl;
}

