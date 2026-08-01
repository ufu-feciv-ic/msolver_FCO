#include "ui/AppUI.h"
#include "imgui.h"
#include "implot.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <iostream>
#include <cmath>

AppUI::AppUI()
{
    GeneratePresetGeometry();
}

AppUI::~AppUI()
{
}

void AppUI::GeneratePresetGeometry()
{
    m_polygonVertices.clear();
    m_rebars.clear();

    if (m_sectionPreset == 0) // Retangular
    {
        float halfB = m_rectB * 0.5f;
        float halfH = m_rectH * 0.5f;

        m_polygonVertices.push_back({ -halfB, -halfH });
        m_polygonVertices.push_back({  halfB, -halfH });
        m_polygonVertices.push_back({  halfB,  halfH });
        m_polygonVertices.push_back({ -halfB,  halfH });

        // Armaduras Inferiores
        float botY = -halfH + m_cover;
        float botStartX = -halfB + m_cover;
        float botEndX = halfB - m_cover;
        if (m_numBotRebars == 1)
        {
            m_rebars.push_back({ 0.0f, botY, m_botRebarDiam });
        }
        else if (m_numBotRebars > 1)
        {
            float stepX = (botEndX - botStartX) / (m_numBotRebars - 1);
            for (int i = 0; i < m_numBotRebars; ++i)
            {
                m_rebars.push_back({ botStartX + i * stepX, botY, m_botRebarDiam });
            }
        }

        // Armaduras Superiores
        float topY = halfH - m_cover;
        float topStartX = -halfB + m_cover;
        float topEndX = halfB - m_cover;
        if (m_numTopRebars == 1)
        {
            m_rebars.push_back({ 0.0f, topY, m_topRebarDiam });
        }
        else if (m_numTopRebars > 1)
        {
            float stepX = (topEndX - topStartX) / (m_numTopRebars - 1);
            for (int i = 0; i < m_numTopRebars; ++i)
            {
                m_rebars.push_back({ topStartX + i * stepX, topY, m_topRebarDiam });
            }
        }
    }
    else if (m_sectionPreset == 1) // Seção T
    {
        float halfBf = m_tBf * 0.5f;
        float halfBw = m_tBw * 0.5f;
        float halfH  = m_tH * 0.5f;
        float yFlange = halfH - m_tHf;

        m_polygonVertices.push_back({ -halfBw, -halfH });
        m_polygonVertices.push_back({  halfBw, -halfH });
        m_polygonVertices.push_back({  halfBw,  yFlange });
        m_polygonVertices.push_back({  halfBf,  yFlange });
        m_polygonVertices.push_back({  halfBf,  halfH });
        m_polygonVertices.push_back({ -halfBf,  halfH });
        m_polygonVertices.push_back({ -halfBf,  yFlange });
        m_polygonVertices.push_back({ -halfBw,  yFlange });

        // Armaduras Inferiores (na Alma)
        float botY = -halfH + m_cover;
        float botStartX = -halfBw + m_cover;
        float botEndX = halfBw - m_cover;
        if (m_numBotRebars > 1)
        {
            float stepX = (botEndX - botStartX) / (m_numBotRebars - 1);
            for (int i = 0; i < m_numBotRebars; ++i)
            {
                m_rebars.push_back({ botStartX + i * stepX, botY, m_botRebarDiam });
            }
        }

        // Armaduras Superiores (na Mesa / Flange)
        float topY = halfH - m_cover;
        float topStartX = -halfBf + m_cover;
        float topEndX = halfBf - m_cover;
        if (m_numTopRebars > 1)
        {
            float stepX = (topEndX - topStartX) / (m_numTopRebars - 1);
            for (int i = 0; i < m_numTopRebars; ++i)
            {
                m_rebars.push_back({ topStartX + i * stepX, topY, m_topRebarDiam });
            }
        }
    }

    UpdatePlotGeometry();
}

void AppUI::UpdatePlotGeometry()
{
    m_polyPlotX.clear();
    m_polyPlotY.clear();
    for (const auto& v : m_polygonVertices)
    {
        m_polyPlotX.push_back(v.x);
        m_polyPlotY.push_back(v.y);
    }
    if (!m_polyPlotX.empty())
    {
        m_polyPlotX.push_back(m_polyPlotX.front());
        m_polyPlotY.push_back(m_polyPlotY.front());
    }

    m_rebarPlotX.clear();
    m_rebarPlotY.clear();
    m_rebarPlotDiam.clear();
    for (const auto& r : m_rebars)
    {
        m_rebarPlotX.push_back(r.x);
        m_rebarPlotY.push_back(r.y);
        m_rebarPlotDiam.push_back(r.diameter);
    }
}

void AppUI::UpdateAndRender()
{
    m_processManager.Update();

    if (m_processManager.IsRunning())
    {
        m_isRunning = true;
        if (m_processManager.HasProgressUpdate())
        {
            auto prog = m_processManager.GetLatestProgress();
            
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
                    m_hasResults = true;
                    m_statusMessage = doc["message"].GetString();
                    m_lastExecutionTime = doc["executionTimeMs"].GetDouble();

                    m_isSafe = doc.HasMember("isSafe") ? doc["isSafe"].GetBool() : false;
                    m_calcArea = doc.HasMember("area") ? doc["area"].GetDouble() : 0.0;
                    m_calcHeight = doc.HasMember("height") ? doc["height"].GetDouble() : 0.0;
                    m_calcCentroidX = doc.HasMember("centroidX") ? doc["centroidX"].GetDouble() : 0.0;
                    m_calcCentroidY = doc.HasMember("centroidY") ? doc["centroidY"].GetDouble() : 0.0;
                    m_calcInertiaX = doc.HasMember("inertiaX") ? doc["inertiaX"].GetDouble() : 0.0;
                    m_calcInertiaY = doc.HasMember("inertiaY") ? doc["inertiaY"].GetDouble() : 0.0;
                    m_calcFcd = doc.HasMember("fcd") ? doc["fcd"].GetDouble() : 0.0;
                    m_calcFyd = doc.HasMember("fyd") ? doc["fyd"].GetDouble() : 0.0;

                    m_envelopeMrdX.clear();
                    m_envelopeMrdY.clear();
                    if (doc.HasMember("envelopeMoments") && doc["envelopeMoments"].IsArray())
                    {
                        for (const auto& pt : doc["envelopeMoments"].GetArray())
                        {
                            if (pt.IsObject() && pt.HasMember("x") && pt.HasMember("y"))
                            {
                                m_envelopeMrdX.push_back(static_cast<float>(pt["x"].GetDouble()));
                                m_envelopeMrdY.push_back(static_cast<float>(pt["y"].GetDouble()));
                            }
                        }
                        if (!m_envelopeMrdX.empty())
                        {
                            m_envelopeMrdX.push_back(m_envelopeMrdX.front());
                            m_envelopeMrdY.push_back(m_envelopeMrdY.front());
                        }
                    }
                }
                else
                {
                    m_statusMessage = "Erro ao executar cálculo no motor worker.";
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
    ImGui::SetNextWindowSize(ImVec2(410, 700), ImGuiCond_FirstUseEver);

    ImGui::Begin("Verificação de Capacidade Resistente (Engine IPC)");

    ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "Parâmetros da Seção Transversal & Materiais");
    ImGui::Separator();
    ImGui::Spacing();

    // 1. Preset de Geometria
    if (ImGui::CollapsingHeader("1. Geometria da Seção Transversal", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const char* presets[] = { "Retangular (B x H)", "Seção T (Flange / Alma)", "Polígono Livre (Vértices Custom)" };
        if (ImGui::Combo("Formato da Seção", &m_sectionPreset, presets, 3))
        {
            GeneratePresetGeometry();
        }

        if (m_sectionPreset == 0) // Retangular
        {
            bool changed = false;
            changed |= ImGui::SliderFloat("Largura B (cm)", &m_rectB, 10.0f, 150.0f, "%.1f cm");
            changed |= ImGui::SliderFloat("Altura H (cm)", &m_rectH, 15.0f, 250.0f, "%.1f cm");
            if (changed) GeneratePresetGeometry();
        }
        else if (m_sectionPreset == 1) // Seção T
        {
            bool changed = false;
            changed |= ImGui::SliderFloat("Largura Mesa bf (cm)", &m_tBf, 20.0f, 200.0f, "%.1f cm");
            changed |= ImGui::SliderFloat("Espessura Mesa hf (cm)", &m_tHf, 5.0f, 50.0f, "%.1f cm");
            changed |= ImGui::SliderFloat("Largura Alma bw (cm)", &m_tBw, 10.0f, 100.0f, "%.1f cm");
            changed |= ImGui::SliderFloat("Altura Total h (cm)", &m_tH, 20.0f, 250.0f, "%.1f cm");
            if (changed) GeneratePresetGeometry();
        }
        else // Polígono Livre
        {
            ImGui::Text("Tabela de Vértices (cm):");
            if (ImGui::Button("+ Adicionar Vértice"))
            {
                m_polygonVertices.push_back({ 0.0f, 0.0f });
                UpdatePlotGeometry();
            }
            ImGui::SameLine();
            if (ImGui::Button("- Remover Último") && !m_polygonVertices.empty())
            {
                m_polygonVertices.pop_back();
                UpdatePlotGeometry();
            }

            for (size_t i = 0; i < m_polygonVertices.size(); ++i)
            {
                ImGui::PushID(static_cast<int>(i));
                if (ImGui::DragFloat2("Ponto", &m_polygonVertices[i].x, 0.5f, -200.0f, 200.0f, "%.1f cm"))
                {
                    UpdatePlotGeometry();
                }
                ImGui::PopID();
            }
        }
    }

    // 2. Configuração de Armaduras
    if (ImGui::CollapsingHeader("2. Disposição das Armaduras (Aço)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (m_sectionPreset != 2) // Retangular ou Seção T
        {
            bool changed = false;
            changed |= ImGui::SliderFloat("Cobrimento (cm)", &m_cover, 1.5f, 7.0f, "%.1f cm");
            changed |= ImGui::SliderInt("Nº Barras Inf (Tracionadas)", &m_numBotRebars, 1, 12);
            changed |= ImGui::SliderFloat("Diâmetro Φ Inf (mm)", &m_botRebarDiam, 6.3f, 40.0f, "%.1f mm");
            changed |= ImGui::SliderInt("Nº Barras Sup (Comprimidas)", &m_numTopRebars, 1, 12);
            changed |= ImGui::SliderFloat("Diâmetro Φ Sup (mm)", &m_topRebarDiam, 6.3f, 40.0f, "%.1f mm");
            if (changed) GeneratePresetGeometry();
        }
        else
        {
            ImGui::Text("Tabela de Armaduras (cm e mm):");
            if (ImGui::Button("+ Adicionar Barra"))
            {
                m_rebars.push_back({ 0.0f, 0.0f, 12.5f });
                UpdatePlotGeometry();
            }
            ImGui::SameLine();
            if (ImGui::Button("- Remover Última") && !m_rebars.empty())
            {
                m_rebars.pop_back();
                UpdatePlotGeometry();
            }

            for (size_t i = 0; i < m_rebars.size(); ++i)
            {
                ImGui::PushID(1000 + static_cast<int>(i));
                ImGui::DragFloat2("Pos (x,y)", &m_rebars[i].x, 0.5f, -200.0f, 200.0f, "%.1f cm");
                ImGui::SameLine();
                if (ImGui::DragFloat("Φ (mm)", &m_rebars[i].diameter, 0.5f, 5.0f, 40.0f, "%.1f mm"))
                {
                    UpdatePlotGeometry();
                }
                ImGui::PopID();
            }
        }
    }

    // 3. Propriedades dos Materiais
    if (ImGui::CollapsingHeader("3. Propriedades dos Materiais (NBR 6118)"))
    {
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "Concreto Armado:");
        ImGui::SliderFloat("fck (MPa)", &m_fck, 15.0f, 90.0f, "%.0f MPa");
        ImGui::SliderFloat("γc (Concreto)", &m_gammaC, 1.0f, 2.0f, "%.2f");
        const char* cModels[] = { "ABNT NBR 6118:2014", "ABNT NBR 6118:2023" };
        ImGui::Combo("Modelo Concreto", &m_concreteModel, cModels, 2);

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "Aço Passivo:");
        ImGui::SliderFloat("fyk (MPa)", &m_fyk, 250.0f, 600.0f, "%.0f MPa");
        ImGui::SliderFloat("γs (Aço)", &m_gammaS, 1.0f, 2.0f, "%.2f");
        ImGui::SliderFloat("Es (GPa)", &m_Es, 150.0f, 250.0f, "%.0f GPa");
    }

    // 4. Solicitação (Esforços Solicitantes)
    if (ImGui::CollapsingHeader("4. Esforços Solicitantes de Projeto", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat("Força Normal Nsd (kN)", &m_Nsd, 5.0f, -5000.0f, 5000.0f, "%.1f kN");
        ImGui::TextDisabled("(Normal: + Tração, - Compressão)");
        ImGui::DragFloat("Momento Msdx (kN.m)", &m_Msdx, 1.0f, -2000.0f, 2000.0f, "%.1f kN.m");
        ImGui::DragFloat("Momento Msdy (kN.m)", &m_Msdy, 1.0f, -2000.0f, 2000.0f, "%.1f kN.m");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (m_isRunning)
    {
        ImGui::BeginDisabled();
        ImGui::Button("Calculando Diagrama de Interação...", ImVec2(-1, 42));
        ImGui::EndDisabled();
    }
    else
    {
        if (ImGui::Button("CALCULAR CAPACIDADE RESISTENTE (WORKER)", ImVec2(-1, 42)))
        {
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
            writer.StartObject();

            writer.Key("polygon");
            writer.StartArray();
            for (const auto& v : m_polygonVertices)
            {
                writer.StartObject();
                writer.Key("x"); writer.Double(v.x);
                writer.Key("y"); writer.Double(v.y);
                writer.EndObject();
            }
            writer.EndArray();

            writer.Key("rebars");
            writer.StartArray();
            for (const auto& r : m_rebars)
            {
                writer.StartObject();
                writer.Key("x"); writer.Double(r.x);
                writer.Key("y"); writer.Double(r.y);
                writer.Key("diameter"); writer.Double(r.diameter);
                writer.EndObject();
            }
            writer.EndArray();

            writer.Key("fck"); writer.Double(m_fck);
            writer.Key("gammaC"); writer.Double(m_gammaC);
            writer.Key("concreteModel"); writer.Int(m_concreteModel);

            writer.Key("fyk"); writer.Double(m_fyk);
            writer.Key("gammaS"); writer.Double(m_gammaS);
            writer.Key("Es"); writer.Double(m_Es);

            writer.Key("Nsd"); writer.Double(m_Nsd);
            writer.Key("Msdx"); writer.Double(m_Msdx);
            writer.Key("Msdy"); writer.Double(m_Msdy);

            writer.EndObject();

            std::string workerPath = WorkerProcessManager::ResolveWorkerPath("analysis-worker.exe");

            if (!m_processManager.StartWorker(workerPath, sb.GetString()))
            {
                m_isRunning = false;
                m_statusMessage = "Erro ao iniciar worker backend: " + workerPath;
            }
            else
            {
                m_isRunning = true;
                m_progress = 0.0f;
                m_statusMessage = "Worker iniciado. Calculando curva envoltória Mrdx x Mrdy...";
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
        ImGui::Text("Tempo do Motor Engine: %.2f ms", m_lastExecutionTime);
    }

    ImGui::End();
}

void AppUI::RenderPlotPanel()
{
    ImGui::SetNextWindowPos(ImVec2(430, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(830, 700), ImGuiCond_FirstUseEver);

    ImGui::Begin("Visualização & Diagramas de Interação (ImPlot)");

    if (ImGui::BeginTabBar("ResultTabs"))
    {
        // -------------------------------------------------------------
        // ABA 1: Diagrama de Interação Biaxial (Mrdx x Mrdy)
        // -------------------------------------------------------------
        if (ImGui::BeginTabItem("Diagrama de Interação Biaxial (Mrdx x Mrdy)"))
        {
            if (m_hasResults)
            {
                if (m_isSafe)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.3f, 1.0f));
                    ImGui::Text("[ SEGURA ] A Seção RESISTE aos esforços solicitantes Msd = (%.1f, %.1f) kN.m para Nsd = %.1f kN.",
                        m_Msdx, m_Msdy, m_Nsd);
                    ImGui::PopStyleColor();
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.2f, 1.0f));
                    ImGui::Text("[ INSEGURA ] A Seção NÃO RESISTE! O ponto solicitante Msd = (%.1f, %.1f) kN.m está fora da envoltória.",
                        m_Msdx, m_Msdy);
                    ImGui::PopStyleColor();
                }
            }

            if (ImPlot::BeginPlot("Envoltória de Momentos Resistentes (Mrd,x x Mrd,y)", ImVec2(-1, -1)))
            {
                ImPlot::SetupAxes("Mrd,x / Msd,x (kN.m)", "Mrd,y / Msd,y (kN.m)");

                if (!m_envelopeMrdX.empty() && !m_envelopeMrdY.empty())
                {
                    ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), 2.5f);
                    ImPlot::PlotLine("Envoltória Mrd (kN.m)", m_envelopeMrdX.data(), m_envelopeMrdY.data(),
                        static_cast<int>(m_envelopeMrdX.size()));
                }

                // Ponto Solicitante Msd
                float msdX = m_Msdx;
                float msdY = m_Msdy;
                if (m_isSafe)
                {
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Cross, 12.0f, ImVec4(0.1f, 1.0f, 0.2f, 1.0f), 3.0f);
                    ImPlot::PlotScatter("Solicitante Msd (Segura)", &msdX, &msdY, 1);
                }
                else
                {
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Cross, 12.0f, ImVec4(1.0f, 0.2f, 0.2f, 1.0f), 3.0f);
                    ImPlot::PlotScatter("Solicitante Msd (Insegura)", &msdX, &msdY, 1);
                }

                ImPlot::EndPlot();
            }

            ImGui::EndTabItem();
        }

        // -------------------------------------------------------------
        // ABA 2: Geometria 2D da Seção Transversal e Armaduras
        // -------------------------------------------------------------
        if (ImGui::BeginTabItem("Geometria 2D & Disposição das Barras"))
        {
            if (ImPlot::BeginPlot("Seção Transversal de Concreto e Armaduras", ImVec2(-1, -1), ImPlotFlags_Equal))
            {
                ImPlot::SetupAxes("Eixo X (cm)", "Eixo Y (cm)");

                // Polígono de Concreto
                if (!m_polyPlotX.empty() && !m_polyPlotY.empty())
                {
                    ImPlot::SetNextLineStyle(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), 3.0f);
                    ImPlot::PlotLine("Contorno Concreto", m_polyPlotX.data(), m_polyPlotY.data(),
                        static_cast<int>(m_polyPlotX.size()));
                }

                // Barras de Armadura
                if (!m_rebarPlotX.empty() && !m_rebarPlotY.empty())
                {
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 8.0f, ImVec4(1.0f, 0.4f, 0.1f, 1.0f), 2.0f);
                    ImPlot::PlotScatter("Barras de Aço", m_rebarPlotX.data(), m_rebarPlotY.data(),
                        static_cast<int>(m_rebarPlotX.size()));
                }

                ImPlot::EndPlot();
            }

            ImGui::EndTabItem();
        }

        // -------------------------------------------------------------
        // ABA 3: Relatório Numérico e Propriedades da Seção
        // -------------------------------------------------------------
        if (ImGui::BeginTabItem("Relatório & Propriedades Numéricas"))
        {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Resumo das Propriedades Geométricas e Resistentes");
            ImGui::Separator();
            ImGui::Spacing();

            if (m_hasResults)
            {
                if (ImGui::BeginTable("PropertiesTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                {
                    ImGui::TableSetupColumn("Propriedade");
                    ImGui::TableSetupColumn("Valor Calculado pelo Engine");
                    ImGui::TableHeadersRow();

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("Área da Seção (A)");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%.2f cm²", m_calcArea);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("Altura Total (H)");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%.2f cm", m_calcHeight);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("Centroide (Xcg, Ycg)");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("(%.2f, %.2f) cm", m_calcCentroidX, m_calcCentroidY);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("Inércia Ixx");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%.2f cm⁴", m_calcInertiaX);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("Inércia Iyy");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%.2f cm⁴", m_calcInertiaY);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("Resistência do Concreto fcd");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%.2f MPa", m_calcFcd);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("Tensão de Escoamento do Aço fyd");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%.2f MPa", m_calcFyd);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("Status de Segurança");
                    ImGui::TableSetColumnIndex(1);
                    if (m_isSafe) ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.3f, 1.0f), "SEGURA (OK)");
                    else ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.2f, 1.0f), "INSEGURA (FALHA)");

                    ImGui::EndTable();
                }
            }
            else
            {
                ImGui::TextWrapped("Clique em 'CALCULAR CAPACIDADE RESISTENTE' no painel à esquerda para executar o motor e visualizar o relatório completo.");
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

