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
