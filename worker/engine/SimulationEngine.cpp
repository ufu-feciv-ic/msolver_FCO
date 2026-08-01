#include "engine/SimulationEngine.h"
#include "engine/Section.h"
#include "engine/Polygon.h"
#include "engine/Reinforcement.h"
#include "engine/ConcreteProperties.h"
#include "engine/SteelProperties.h"

#include <chrono>
#include <iostream>

SimulationEngine::SimulationResult SimulationEngine::RunSimulation(const SimulationInput& input, ProgressCallback onProgress)
{
    const auto startTime = std::chrono::high_resolution_clock::now();

    if (onProgress) onProgress(0.1f, "Montando geometria e propriedades da seção de concreto...");

    // 1. Polígono de Concreto
    Polygon polygon;
    for (const auto& pt : input.polygonVertices)
    {
        polygon.addVertice(pt.getX(), pt.getY());
    }

    // 2. Armaduras de Aço
    Reinforcement reinforcement;
    for (const auto& rebar : input.rebars)
    {
        reinforcement.addReinforcement(rebar.x, rebar.y, rebar.diameter);
    }

    // 3. Materiais
    ConcreteProperties concrete;
    StressStrainConcreteModelType cModel = (input.concreteModelType == 0)
        ? StressStrainConcreteModelType::PARABOLA_RECTANGLE_NBR6118_2014
        : StressStrainConcreteModelType::PARABOLA_RECTANGLE_NBR6118_2023;
    concrete.setParameters(cModel, input.fck, input.gammaC);

    SteelProperties steel;
    steel.setParameters(StressStrainSteelModelType::PASSIVE_REINFORCEMENT, input.fyk, input.gammaS, input.Es);

    // 4. Instanciar Seção Transversal
    Section section;
    section.defineGeometry(polygon, reinforcement);
    section.defineMaterials(concrete, steel);
    section.updateGeometricProperties();

    if (onProgress) onProgress(0.3f, "Calculando diagrama de interação e envoltória de momentos resistentes (Mrdx, Mrdy)...");

    // 5. Calcular Envoltória Resistente para a Normal Nsd informada
    section.computeEnvelope(input.Nsd);

    if (onProgress) onProgress(0.85f, "Verificando equilíbrio das solicitações e critérios de segurança...");

    // 6. Verificar Segurança contra Msdx e Msdy
    Point momentDemand(input.Msdx, input.Msdy);
    bool isSafe = section.isMomentSafe(momentDemand);

    const auto& workingPoly = section.getWorkingPolygon();

    SimulationResult result;
    result.success = true;
    result.area = workingPoly.getPolygonArea();
    result.height = workingPoly.getPolygonHeight();
    result.centroid = workingPoly.getGeometricCenter();
    result.inertiaX = workingPoly.getInertiaX_cg();
    result.inertiaY = workingPoly.getInertiaY_cg();
    result.fcd = concrete.getFcd();
    result.fyd = steel.getFyd();

    result.Nsd = input.Nsd;
    result.Msdx = input.Msdx;
    result.Msdy = input.Msdy;

    result.envelopeMoments = section.getEnvelopeMoments();
    result.isSafe = isSafe;
    result.polygonVertices = input.polygonVertices;
    result.rebars = input.rebars;

    const auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    if (isSafe)
    {
        result.message = "Verificação concluída: SEÇÃO SEGURA (Esforços solicitantes internos à capacidade resistente).";
    }
    else
    {
        result.message = "Verificação concluída: SEÇÃO INSEGURA (Esforços solicitantes superam a capacidade resistente da seção).";
    }

    if (onProgress) onProgress(1.0f, "Cálculo concluído!");

    return result;
}

SimulationEngine::SizingResult SimulationEngine::RunSizingSimulation(const SimulationInput& input, ProgressCallback onProgress)
{
    const auto startTime = std::chrono::high_resolution_clock::now();

    if (onProgress) onProgress(0.05f, "Iniciando algoritmo de dimensionamento da área de aço (Bisseção)...");

    SizingResult result;

    // 1. Área de aço base inicial das armaduras de entrada
    double baseSteelArea = 0.0;
    for (const auto& rebar : input.rebars)
    {
        double r_cm = (rebar.diameter / 10.0) * 0.5;
        baseSteelArea += M_PI * r_cm * r_cm;
    }

    if (baseSteelArea <= 0.0)
    {
        result.success = false;
        result.message = "Erro: Nenhuma barra de armadura definida para dimensionar.";
        return result;
    }

    result.initialSteelArea = baseSteelArea;

    // Função interna que executa a simulação para um fator de escala k_scale
    auto evaluateScale = [&](double k_scale) -> SimulationResult
    {
        SimulationInput scaledInput = input;
        double diameterMult = std::sqrt(k_scale);
        for (auto& rebar : scaledInput.rebars)
        {
            rebar.diameter = rebar.diameter * diameterMult;
        }
        return RunSimulation(scaledInput, nullptr);
    };

    // 2. Definir limites de busca do fator de escala k_s
    double k_low = 0.001; // Próximo de zero
    double k_high = 1.0;

    // Testar se a armadura inicial k=1.0 já é segura
    SimulationResult resHigh = evaluateScale(k_high);

    int expandCount = 0;
    while (!resHigh.isSafe && expandCount < 15)
    {
        k_high *= 1.8;
        resHigh = evaluateScale(k_high);
        expandCount++;
    }

    if (!resHigh.isSafe)
    {
        result.success = false;
        result.message = "Dimensionamento falhou: A seção necessita de taxa de armadura superior ao limite máximo razoável (>4% Ac).";
        return result;
    }

    // 3. Loop da Bisseção para encontrar a área exata As,req
    double tolAreaCm2 = 0.01; // Tolerância de 0.01 cm² de aço
    int maxIter = 30;
    int iter = 0;

    double k_mid = (k_low + k_high) * 0.5;

    while ((k_high - k_low) * baseSteelArea > tolAreaCm2 && iter < maxIter)
    {
        iter++;
        k_mid = (k_low + k_high) * 0.5;

        if (onProgress)
        {
            float prog = 0.1f + 0.85f * (static_cast<float>(iter) / maxIter);
            std::string status = "Iteração " + std::to_string(iter) + ": testando As = " +
                std::to_string(k_mid * baseSteelArea) + " cm²...";
            onProgress(prog, status);
        }

        SimulationResult resMid = evaluateScale(k_mid);

        if (resMid.isSafe)
        {
            k_high = k_mid; // Reduzir armadura se for segura
        }
        else
        {
            k_low = k_mid;  // Aumentar armadura se for insegura
        }
    }

    // 4. Compilar Resultados Finais
    double finalScale = k_high;
    SimulationResult finalVerification = evaluateScale(finalScale);

    result.success = true;
    result.scaleFactor = finalScale;
    result.requiredSteelArea = finalScale * baseSteelArea;
    result.iterationsCount = iter;

    double Ac = finalVerification.area; // cm²
    double H = finalVerification.height; // cm

    // Estimar largura B a partir de Ac e H se retangular/T
    double B = (H > 0.0) ? (Ac / H) : H;

    double fcd_kN_cm2 = finalVerification.fcd * 0.1; // 1 MPa = 0.1 kN/cm²
    double fyd_kN_cm2 = finalVerification.fyd * 0.1;

    result.steelRatioPercent = (Ac > 0.0) ? (100.0 * result.requiredSteelArea / Ac) : 0.0;
    result.omegaMechanicalRatio = (Ac > 0.0 && fcd_kN_cm2 > 0.0) ?
        (result.requiredSteelArea * fyd_kN_cm2) / (Ac * fcd_kN_cm2) : 0.0;

    result.nuReducedNormal = (Ac > 0.0 && fcd_kN_cm2 > 0.0) ?
        (input.Nsd / (Ac * fcd_kN_cm2)) : 0.0;

    result.muXReducedMoment = (Ac > 0.0 && H > 0.0 && fcd_kN_cm2 > 0.0) ?
        (input.Msdx * 100.0 / (Ac * H * fcd_kN_cm2)) : 0.0;

    result.muYReducedMoment = (Ac > 0.0 && B > 0.0 && fcd_kN_cm2 > 0.0) ?
        (input.Msdy * 100.0 / (Ac * B * fcd_kN_cm2)) : 0.0;

    // Diâmetros de armadura resultantes
    double diamMult = std::sqrt(finalScale);
    result.sizedRebars = input.rebars;
    for (auto& r : result.sizedRebars)
    {
        r.diameter = r.diameter * diamMult;
    }

    result.verificationResult = finalVerification;

    const auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    result.message = "Dimensionamento concluído com sucesso em " + std::to_string(iter) +
        " iterações! Área necessária As,req = " + std::to_string(result.requiredSteelArea) + " cm² (ω = " +
        std::to_string(result.omegaMechanicalRatio) + ").";

    if (onProgress) onProgress(1.0f, "Dimensionamento concluído!");

    return result;
}

