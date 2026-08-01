#pragma once

#include <string>
#include <vector>

#include "ipc/WorkerProcessManager.h"

class AppUI
{
public:
    AppUI();
    ~AppUI();

    void UpdateAndRender();

private:
    void RenderControlPanel();
    void RenderPlotPanel();

    WorkerProcessManager m_processManager;

    int m_matrixSize = 15;
    float m_loadFactor = 1.5f;

    bool m_isRunning = false;
    float m_progress = 0.0f;
    std::string m_statusMessage = "Aguardando início...";

    std::vector<float> m_xCoords;
    std::vector<float> m_yCoords;
    double m_lastExecutionTime = 0.0;
};
