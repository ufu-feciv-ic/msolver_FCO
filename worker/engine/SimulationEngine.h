#pragma once

#include <string>
#include <vector>
#include <functional>

#include "Point.h"

class SimulationEngine
{
public:
    struct RebarBarInput
    {
        double x = 0.0;
        double y = 0.0;
        double diameter = 10.0; // mm
    };

    struct SimulationInput
    {
        // Vértices do Polígono em cm
        std::vector<Point> polygonVertices;

        // Armaduras (posição em cm, diâmetro em mm)
        std::vector<RebarBarInput> rebars;

        // Propriedades do Concreto
        double fck = 30.0; // MPa
        double gammaC = 1.4;
        int concreteModelType = 1; // 0 = NBR 6118:2014, 1 = NBR 6118:2023

        // Propriedades do Aço
        double fyk = 500.0; // MPa
        double gammaS = 1.15;
        double Es = 210.0; // GPa

        // Esforços Solicitantes de Projeto
        double Nsd = 0.0;  // kN (Esforço Normal)
        double Msdx = 0.0; // kN.m (Momento em X)
        double Msdy = 0.0; // kN.m (Momento em Y)
    };

    struct SimulationResult
    {
        bool success = false;
        double executionTimeMs = 0.0;
        std::string message;

        // Propriedades Geométricas da Seção
        double area = 0.0;     // cm²
        double height = 0.0;   // cm
        Point centroid;        // cm
        double inertiaX = 0.0; // cm⁴
        double inertiaY = 0.0; // cm⁴

        // Parâmetros dos Materiais
        double fcd = 0.0; // MPa
        double fyd = 0.0; // MPa

        // Esforços Solicitantes
        double Nsd = 0.0;  // kN
        double Msdx = 0.0; // kN.m
        double Msdy = 0.0; // kN.m

        // Envoltória de Momentos Resistentes (Mrd_x, Mrd_y) em kN.m
        std::vector<Point> envelopeMoments;

        // Verificação de Segurança
        bool isSafe = false;

        // Echo da Geometria para Pré-visualização na UI
        std::vector<Point> polygonVertices;
        std::vector<RebarBarInput> rebars;
    };

    using ProgressCallback = std::function<void(float progress, const std::string& status)>;

    SimulationResult RunSimulation(const SimulationInput& input, ProgressCallback onProgress);
};
