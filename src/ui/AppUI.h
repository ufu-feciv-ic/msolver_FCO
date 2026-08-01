#pragma once

#include <string>
#include <vector>

#include "ipc/WorkerProcessManager.h"
#include "engine/Point.h"

struct UIRebarBar
{
    float x = 0.0f; // cm
    float y = 0.0f; // cm
    float diameter = 12.5f; // mm
};

struct UIPolygonVertex
{
    float x = 0.0f; // cm
    float y = 0.0f; // cm
};

class AppUI
{
public:
    AppUI();
    ~AppUI();

    void UpdateAndRender();

private:
    void RenderControlPanel();
    void RenderPlotPanel();
    void GeneratePresetGeometry();
    void UpdatePlotGeometry();

    WorkerProcessManager m_processManager;

    // Preset de Geometria: 0 = Retangular, 1 = Seção T, 2 = Polígono Livre
    int m_sectionPreset = 0;

    // Parâmetros Retangular (cm)
    float m_rectB = 20.0f;
    float m_rectH = 50.0f;

    // Parâmetros Seção T (cm)
    float m_tBf = 60.0f;
    float m_tHf = 12.0f;
    float m_tBw = 20.0f;
    float m_tH  = 50.0f;

    // Configuração de Armadura Automática
    float m_cover = 3.0f; // cm
    int m_numBotRebars = 4;
    float m_botRebarDiam = 16.0f; // mm
    int m_numTopRebars = 2;
    float m_topRebarDiam = 12.5f; // mm

    // Vértices do Polígono e Armaduras
    std::vector<UIPolygonVertex> m_polygonVertices;
    std::vector<UIRebarBar> m_rebars;

    // Propriedades do Concreto
    float m_fck = 30.0f; // MPa
    float m_gammaC = 1.4f;
    int m_concreteModel = 1; // 0 = NBR 6118:2014, 1 = NBR 6118:2023

    // Propriedades do Aço
    float m_fyk = 500.0f; // MPa
    float m_gammaS = 1.15f;
    float m_Es = 210.0f; // GPa

    // Esforços Solicitantes de Projeto
    float m_Nsd = 0.0f;    // kN
    float m_Msdx = 60.0f;  // kN.m
    float m_Msdy = 15.0f;  // kN.m

    // Estado da Execução IPC
    bool m_isRunning = false;
    float m_progress = 0.0f;
    std::string m_statusMessage = "Pronto para calcular.";

    // Resultados Retornados pelo Worker Engine
    bool m_hasResults = false;
    bool m_isSafe = false;
    double m_lastExecutionTime = 0.0;

    double m_calcArea = 0.0;
    double m_calcHeight = 0.0;
    double m_calcCentroidX = 0.0;
    double m_calcCentroidY = 0.0;
    double m_calcInertiaX = 0.0;
    double m_calcInertiaY = 0.0;
    double m_calcFcd = 0.0;
    double m_calcFyd = 0.0;

    // Curva Envoltória de Momentos (ImPlot)
    std::vector<float> m_envelopeMrdX;
    std::vector<float> m_envelopeMrdY;

    // Geometria Renderizada no Gráfico 2D
    std::vector<float> m_polyPlotX;
    std::vector<float> m_polyPlotY;
    std::vector<float> m_rebarPlotX;
    std::vector<float> m_rebarPlotY;
    std::vector<float> m_rebarPlotDiam;
};
