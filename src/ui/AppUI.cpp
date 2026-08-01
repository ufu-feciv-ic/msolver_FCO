#include "ui/AppUI.h"
#include "imgui.h"
#include "implot.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <iostream>

AppUI::AppUI()
{
}

AppUI::~AppUI()
{
}

void AppUI::UpdateAndRender()
{
    // Atualizar estado do Worker Process IPC
    m_processManager.Update();

    if (m_processManager.IsRunning())
    {
        m_isRunning = true;
        if (m_processManager.HasProgressUpdate())
        {
            auto prog = m_processManager.GetLatestProgress();
            
            // Tenta parsear JSON de progresso
            rapidjson::Document doc;
            doc.Parse(prog.status.c_str());
            if (!doc.HasParseError())
            {
                if (doc.HasMember("progress") && doc["progress"].IsDouble())
                    m_progress = static_cast<float>(doc["progress"].GetDouble());
                if (doc.HasMember("status") && doc["status"].IsString())
                    m_statusMessage = doc["status"].GetString();
            }
        }
    }
    else
    {
        if (m_isRunning)
        {
            m_isRunning = false;
            if (m_processManager.HasNewResult())
            {
                std::string resultJson = m_processManager.GetLatestResult();
                
                rapidjson::Document doc;
                doc.Parse(resultJson.c_str());
                if (!doc.HasParseError() && doc.HasMember("success") && doc["success"].GetBool())
                {
                    m_statusMessage = doc["message"].GetString();
                    m_lastExecutionTime = doc["executionTimeMs"].GetDouble();

                    if (doc.HasMember("solution") && doc["solution"].IsArray())
                    {
                        const auto& arr = doc["solution"].GetArray();
                        m_xCoords.clear();
                        m_yCoords.clear();
                        for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
                        {
                            m_xCoords.push_back(static_cast<float>(i + 1));
                            m_yCoords.push_back(static_cast<float>(arr[i].GetDouble()));
                        }
                    }
                }
                else
                {
                    m_statusMessage = "Erro ao executar cálculo no motor.";
                }
            }
        }
    }

    RenderControlPanel();
    RenderPlotPanel();
}

void AppUI::RenderControlPanel()
{
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(380, 680), ImGuiCond_FirstUseEver);

    ImGui::Begin("Painel de Controle do Motor (Engine IPC)");

    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Parâmetros do Motor Computacional");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::SliderInt("Tamanho da Matriz (Eigen)", &m_matrixSize, 5, 100);
    ImGui::SliderFloat("Fator de Carga", &m_loadFactor, 0.1f, 10.0f, "%.2f");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (m_isRunning)
    {
        ImGui::BeginDisabled();
        ImGui::Button("Calculando...", ImVec2(-1, 40));
        ImGui::EndDisabled();
    }
    else
    {
        if (ImGui::Button("EXECUTAR MOTOR (WORKER)", ImVec2(-1, 40)))
        {
            // Criar JSON de requisição
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
            writer.StartObject();
            writer.Key("matrixSize");
            writer.Int(m_matrixSize);
            writer.Key("loadFactor");
            writer.Double(m_loadFactor);
            writer.EndObject();

            std::string workerPath = WorkerProcessManager::ResolveWorkerPath("analysis-worker.exe");

            if (!m_processManager.StartWorker(workerPath, sb.GetString()))
            {
                m_isRunning = false;
                m_statusMessage = "Erro ao iniciar worker: " + workerPath;
            }
            else
            {
                m_isRunning = true;
                m_progress = 0.0f;
                m_statusMessage = "Worker iniciado, aguardando cálculo...";
            }
        }
    }

    ImGui::Spacing();
    ImGui::TextWrapped("Status: %s", m_statusMessage.c_str());
    if (m_isRunning)
    {
        ImGui::ProgressBar(m_progress, ImVec2(-1, 20));
    }

    if (m_lastExecutionTime > 0.0)
    {
        ImGui::Text("Tempo do Worker: %.2f ms", m_lastExecutionTime);
    }

    ImGui::End();
}

void AppUI::RenderPlotPanel()
{
    ImGui::SetNextWindowPos(ImVec2(400, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(860, 680), ImGuiCond_FirstUseEver);

    ImGui::Begin("Gráficos de Resposta (ImPlot)");

    if (!m_xCoords.empty() && !m_yCoords.empty())
    {
        if (ImPlot::BeginPlot("Solução do Sistema Linear Ax = b (Eigen)", ImVec2(-1, -1)))
        {
            ImPlot::SetupAxes("Índice do Nó (x)", "Deslocamento / Resposta (y)");
            ImPlot::PlotLine("Vetor Solução x", m_xCoords.data(), m_yCoords.data(), static_cast<int>(m_xCoords.size()));
            ImPlot::PlotScatter("Pontos Discretos", m_xCoords.data(), m_yCoords.data(), static_cast<int>(m_xCoords.size()));
            ImPlot::EndPlot();
        }
    }
    else
    {
        ImGui::TextWrapped("Nenhum dado calculado ainda. Clique em 'EXECUTAR MOTOR (WORKER)' para calcular com Eigen e plotar o gráfico.");
    }

    ImGui::End();
}
