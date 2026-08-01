#include "ui/AppUI.h"
#include "imgui.h"
#include "implot.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <iostream>
#include <cmath>
#include <algorithm>

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

void AppUI::ApplyCustomTheme()
{
    if (m_themeInitialized) return;
    m_themeInitialized = true;

    ImGuiStyle& style = ImGui::GetStyle();
    
    style.WindowRounding    = 6.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 4.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 5.0f;

    style.WindowPadding     = ImVec2(12.0f, 12.0f);
    style.FramePadding      = ImVec2(8.0f, 5.0f);
    style.ItemSpacing       = ImVec2(8.0f, 8.0f);
    style.ItemInnerSpacing  = ImVec2(6.0f, 4.0f);
    style.ScrollbarSize     = 12.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]             = ImVec4(0.09f, 0.10f, 0.13f, 1.00f);
    colors[ImGuiCol_ChildBg]              = ImVec4(0.12f, 0.13f, 0.17f, 1.00f);
    colors[ImGuiCol_PopupBg]              = ImVec4(0.12f, 0.14f, 0.19f, 0.98f);
    colors[ImGuiCol_Border]               = ImVec4(0.20f, 0.23f, 0.30f, 0.60f);
    colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.15f, 0.17f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.22f, 0.26f, 0.35f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.26f, 0.32f, 0.44f, 1.00f);
    colors[ImGuiCol_TitleBg]              = ImVec4(0.08f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.11f, 0.13f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.08f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_MenuBarBg]            = ImVec4(0.11f, 0.12f, 0.16f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.08f, 0.09f, 0.12f, 0.50f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.22f, 0.26f, 0.35f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.36f, 0.48f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.36f, 0.44f, 0.58f, 1.00f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.00f, 0.75f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.00f, 0.65f, 0.95f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.20f, 0.80f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]               = ImVec4(0.16f, 0.36f, 0.62f, 1.00f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.22f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.26f, 0.56f, 0.92f, 1.00f);
    colors[ImGuiCol_Header]               = ImVec4(0.18f, 0.22f, 0.30f, 1.00f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.24f, 0.30f, 0.42f, 1.00f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.28f, 0.36f, 0.50f, 1.00f);
    colors[ImGuiCol_Separator]            = ImVec4(0.20f, 0.24f, 0.32f, 0.80f);
    colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.00f, 0.70f, 1.00f, 0.80f);
    colors[ImGuiCol_SeparatorActive]      = ImVec4(0.00f, 0.80f, 1.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip]           = ImVec4(0.20f, 0.24f, 0.32f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.00f, 0.70f, 1.00f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.00f, 0.80f, 1.00f, 0.95f);
    colors[ImGuiCol_Tab]                  = ImVec4(0.12f, 0.14f, 0.19f, 1.00f);
    colors[ImGuiCol_TabHovered]           = ImVec4(0.22f, 0.40f, 0.68f, 1.00f);
    colors[ImGuiCol_TabActive]            = ImVec4(0.16f, 0.36f, 0.62f, 1.00f);
    colors[ImGuiCol_TabUnfocused]         = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.14f, 0.16f, 0.22f, 1.00f);
    colors[ImGuiCol_PlotLines]            = ImVec4(0.00f, 0.75f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]     = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]        = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]        = ImVec4(0.15f, 0.18f, 0.25f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]    = ImVec4(0.20f, 0.24f, 0.32f, 1.00f);
    colors[ImGuiCol_TableBorderLight]     = ImVec4(0.16f, 0.19f, 0.26f, 1.00f);
    colors[ImGuiCol_TableRowBg]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]        = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);
    colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.00f, 0.50f, 0.90f, 0.35f);
}

void AppUI::ApplyPresetGeometry(int presetType)
{
    if (presetType == 0) // Viga 20x50
    {
        m_sectionPreset = 0;
        m_rectB = 20.0f;
        m_rectH = 50.0f;
        m_cover = 3.0f;
        m_numBotRebars = 4;
        m_botRebarDiam = 16.0f;
        m_numTopRebars = 2;
        m_topRebarDiam = 12.5f;
    }
    else if (presetType == 1) // Seção T
    {
        m_sectionPreset = 1;
        m_tBf = 60.0f;
        m_tHf = 12.0f;
        m_tBw = 20.0f;
        m_tH = 50.0f;
        m_cover = 3.0f;
        m_numBotRebars = 4;
        m_botRebarDiam = 20.0f;
        m_numTopRebars = 3;
        m_topRebarDiam = 12.5f;
    }
    else if (presetType == 2) // Pilar 30x60
    {
        m_sectionPreset = 0;
        m_rectB = 30.0f;
        m_rectH = 60.0f;
        m_cover = 3.0f;
        m_numBotRebars = 4;
        m_botRebarDiam = 20.0f;
        m_numTopRebars = 4;
        m_topRebarDiam = 20.0f;
        m_Nsd = -400.0f;
        m_Msdx = 80.0f;
        m_Msdy = 25.0f;
    }
    GeneratePresetGeometry();
    if (m_autoCalculate) m_needsRecalculation = true;
}

void AppUI::ApplyLoadingPreset(int loadPreset)
{
    if (loadPreset == 0) // Flexão Simples (Msdx = 80)
    {
        m_Nsd = 0.0f;
        m_Msdx = 80.0f;
        m_Msdy = 0.0f;
    }
    else if (loadPreset == 1) // Flexão Biaxial (Msdx = 60, Msdy = 25)
    {
        m_Nsd = 0.0f;
        m_Msdx = 60.0f;
        m_Msdy = 25.0f;
    }
    else if (loadPreset == 2) // Compressão Centrada (Nsd = -500)
    {
        m_Nsd = -500.0f;
        m_Msdx = 0.0f;
        m_Msdy = 0.0f;
    }
    else if (loadPreset == 3) // Flexo-Compressão Biaxial (Nsd = -300, Msdx = 50, Msdy = 20)
    {
        m_Nsd = -300.0f;
        m_Msdx = 50.0f;
        m_Msdy = 20.0f;
    }
    if (m_autoCalculate) m_needsRecalculation = true;
}

void AppUI::TriggerWorkerCalculation()
{
    if (m_isRunning) return;

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
        m_statusMessage = "Calculando envoltória Mrdx x Mrdy...";
    }
    m_needsRecalculation = false;
}

void AppUI::TriggerWorkerSizing()
{
    if (m_isRunning) return;

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();

    writer.Key("mode"); writer.String("sizing");

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
        m_statusMessage = "Dimensionando área de aço necessária As (Bisseção)...";
    }
    m_needsRecalculation = false;
}

void AppUI::RenderHeaderBar()
{
    float screenWidth = ImGui::GetIO().DisplaySize.x;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(screenWidth, 46.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 8.0f));
    ImGui::Begin("TopHeaderBar", nullptr, flags);

    ImGui::TextColored(ImVec4(0.00f, 0.75f, 1.00f, 1.00f), "MSOLVER | FCO");
    ImGui::SameLine();
    ImGui::TextDisabled("v2.0 — Análise & Dimensionamento de Concreto Armado");

    ImGui::SameLine(0.0f, 20.0f);

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Presets:");
    ImGui::SameLine();
    if (ImGui::Button("Viga 20x50")) ApplyPresetGeometry(0);
    ImGui::SameLine();
    if (ImGui::Button("Seção T")) ApplyPresetGeometry(1);
    ImGui::SameLine();
    if (ImGui::Button("Pilar 30x60")) ApplyPresetGeometry(2);

    float rightWidth = 520.0f;
    float rightPos = screenWidth - rightWidth;
    if (rightPos > 480.0f) ImGui::SameLine(rightPos);
    else ImGui::SameLine();

    if (ImGui::Checkbox("Cálculo Automático (Live)", &m_autoCalculate))
    {
        if (m_autoCalculate) m_needsRecalculation = true;
    }

    ImGui::SameLine();
    if (m_isRunning)
    {
        ImGui::BeginDisabled();
        ImGui::Button("Calculando...", ImVec2(130, 0));
        ImGui::EndDisabled();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.60f, 0.45f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.75f, 0.55f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.85f, 0.65f, 1.00f));
        if (ImGui::Button("⚡ VERIFICAR", ImVec2(120, 0)))
        {
            TriggerWorkerCalculation();
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.45f, 0.80f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.55f, 0.95f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.24f, 0.65f, 1.00f, 1.00f));
        if (ImGui::Button("🎯 DIMENSIONAR As", ImVec2(140, 0)))
        {
            TriggerWorkerSizing();
        }
        ImGui::PopStyleColor(3);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void AppUI::UpdateAndRender()
{
    ApplyCustomTheme();

    m_processManager.Update();

    if (!m_isRunning && m_needsRecalculation)
    {
        TriggerWorkerCalculation();
    }

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

                    m_isSizingMode = doc.HasMember("isSizing") ? doc["isSizing"].GetBool() : false;
                    if (m_isSizingMode)
                    {
                        m_calcRequiredSteelArea = doc.HasMember("requiredSteelArea") ? doc["requiredSteelArea"].GetDouble() : 0.0;
                        m_calcSteelRatioPercent = doc.HasMember("steelRatioPercent") ? doc["steelRatioPercent"].GetDouble() : 0.0;
                        m_calcOmegaMechanicalRatio = doc.HasMember("omegaMechanicalRatio") ? doc["omegaMechanicalRatio"].GetDouble() : 0.0;
                        m_calcNuReducedNormal = doc.HasMember("nuReducedNormal") ? doc["nuReducedNormal"].GetDouble() : 0.0;
                        m_calcMuXReducedMoment = doc.HasMember("muXReducedMoment") ? doc["muXReducedMoment"].GetDouble() : 0.0;
                        m_calcMuYReducedMoment = doc.HasMember("muYReducedMoment") ? doc["muYReducedMoment"].GetDouble() : 0.0;
                        m_calcSizingIterations = doc.HasMember("iterationsCount") ? doc["iterationsCount"].GetInt() : 0;
                    }

                    if (doc.HasMember("rebars") && doc["rebars"].IsArray())
                    {
                        m_rebars.clear();
                        for (const auto& r : doc["rebars"].GetArray())
                        {
                            if (r.IsObject() && r.HasMember("x") && r.HasMember("y") && r.HasMember("diameter"))
                            {
                                UIRebarBar bar;
                                bar.x = static_cast<float>(r["x"].GetDouble());
                                bar.y = static_cast<float>(r["y"].GetDouble());
                                bar.diameter = static_cast<float>(r["diameter"].GetDouble());
                                m_rebars.push_back(bar);
                            }
                        }
                        UpdatePlotGeometry();
                    }

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

    RenderHeaderBar();
    RenderControlPanel();
    RenderPlotPanel();
}

void AppUI::RenderControlPanel()
{
    float screenWidth = ImGui::GetIO().DisplaySize.x;
    float screenHeight = ImGui::GetIO().DisplaySize.y;
    float headerHeight = 46.0f;
    float panelWidth = std::clamp(screenWidth * 0.32f, 360.0f, 460.0f);

    ImGui::SetNextWindowPos(ImVec2(0.0f, headerHeight));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, screenHeight - headerHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("LeftControlPanel", nullptr, flags);

    ImGui::TextColored(ImVec4(0.00f, 0.75f, 1.00f, 1.00f), "PAINEL DE PARÂMETROS");
    ImGui::Separator();
    ImGui::Spacing();

    // 1. Geometria da Seção Transversal
    if (ImGui::CollapsingHeader("1. Geometria da Seção", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const char* presets[] = { "Retangular (B x H)", "Seção T (Mesa / Alma)", "Polígono Livre" };
        if (ImGui::Combo("Formato", &m_sectionPreset, presets, 3))
        {
            GeneratePresetGeometry();
            if (m_autoCalculate) m_needsRecalculation = true;
        }

        if (m_sectionPreset == 0) // Retangular
        {
            bool changed = false;
            changed |= ImGui::SliderFloat("Largura B (cm)", &m_rectB, 10.0f, 150.0f, "%.1f cm");
            changed |= ImGui::SliderFloat("Altura H (cm)", &m_rectH, 15.0f, 250.0f, "%.1f cm");
            if (changed)
            {
                GeneratePresetGeometry();
                if (m_autoCalculate) m_needsRecalculation = true;
            }
        }
        else if (m_sectionPreset == 1) // Seção T
        {
            bool changed = false;
            changed |= ImGui::SliderFloat("Mesa bf (cm)", &m_tBf, 20.0f, 200.0f, "%.1f cm");
            changed |= ImGui::SliderFloat("Espessura hf (cm)", &m_tHf, 5.0f, 50.0f, "%.1f cm");
            changed |= ImGui::SliderFloat("Alma bw (cm)", &m_tBw, 10.0f, 100.0f, "%.1f cm");
            changed |= ImGui::SliderFloat("Altura Total h (cm)", &m_tH, 20.0f, 250.0f, "%.1f cm");
            if (changed)
            {
                GeneratePresetGeometry();
                if (m_autoCalculate) m_needsRecalculation = true;
            }
        }
        else // Polígono Livre
        {
            ImGui::Text("Vértices do Polígono (cm):");
            if (ImGui::Button("+ Adicionar Vértice"))
            {
                m_polygonVertices.push_back({ 0.0f, 0.0f });
                UpdatePlotGeometry();
                if (m_autoCalculate) m_needsRecalculation = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("- Remover Último") && !m_polygonVertices.empty())
            {
                m_polygonVertices.pop_back();
                UpdatePlotGeometry();
                if (m_autoCalculate) m_needsRecalculation = true;
            }

            for (size_t i = 0; i < m_polygonVertices.size(); ++i)
            {
                ImGui::PushID(static_cast<int>(i));
                if (ImGui::DragFloat2("Ponto", &m_polygonVertices[i].x, 0.5f, -200.0f, 200.0f, "%.1f cm"))
                {
                    UpdatePlotGeometry();
                    if (m_autoCalculate) m_needsRecalculation = true;
                }
                ImGui::PopID();
            }
        }
    }

    // 2. Disposição das Armaduras
    if (ImGui::CollapsingHeader("2. Disposição das Armaduras (Aço)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (m_sectionPreset != 2)
        {
            bool changed = false;
            changed |= ImGui::SliderFloat("Cobrimento (cm)", &m_cover, 1.5f, 7.0f, "%.1f cm");
            changed |= ImGui::SliderInt("Barras Inf (Tracionadas)", &m_numBotRebars, 1, 12);
            changed |= ImGui::SliderFloat("Φ Inf (mm)", &m_botRebarDiam, 6.3f, 40.0f, "%.1f mm");

            ImGui::TextDisabled("Bitolas Inf:");
            float gaugeList[] = { 8.0f, 10.0f, 12.5f, 16.0f, 20.0f, 25.0f };
            for (float g : gaugeList)
            {
                char btnLabel[16];
                std::snprintf(btnLabel, sizeof(btnLabel), "Φ%.1f", g);
                if (ImGui::Button(btnLabel))
                {
                    m_botRebarDiam = g;
                    changed = true;
                }
                ImGui::SameLine();
            }
            ImGui::NewLine();

            changed |= ImGui::SliderInt("Barras Sup (Comprimidas)", &m_numTopRebars, 1, 12);
            changed |= ImGui::SliderFloat("Φ Sup (mm)", &m_topRebarDiam, 6.3f, 40.0f, "%.1f mm");

            ImGui::TextDisabled("Bitolas Sup:");
            for (float g : gaugeList)
            {
                char btnLabel[16];
                std::snprintf(btnLabel, sizeof(btnLabel), "Φ%.1f##top", g);
                if (ImGui::Button(btnLabel))
                {
                    m_topRebarDiam = g;
                    changed = true;
                }
                ImGui::SameLine();
            }
            ImGui::NewLine();

            if (changed)
            {
                GeneratePresetGeometry();
                if (m_autoCalculate) m_needsRecalculation = true;
            }
        }
        else
        {
            ImGui::Text("Tabela de Armaduras (cm e mm):");
            if (ImGui::Button("+ Adicionar Barra"))
            {
                m_rebars.push_back({ 0.0f, 0.0f, 12.5f });
                UpdatePlotGeometry();
                if (m_autoCalculate) m_needsRecalculation = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("- Remover Última") && !m_rebars.empty())
            {
                m_rebars.pop_back();
                UpdatePlotGeometry();
                if (m_autoCalculate) m_needsRecalculation = true;
            }

            for (size_t i = 0; i < m_rebars.size(); ++i)
            {
                ImGui::PushID(1000 + static_cast<int>(i));
                bool changed = false;
                changed |= ImGui::DragFloat2("Pos (x,y)", &m_rebars[i].x, 0.5f, -200.0f, 200.0f, "%.1f cm");
                ImGui::SameLine();
                changed |= ImGui::DragFloat("Φ (mm)", &m_rebars[i].diameter, 0.5f, 5.0f, 40.0f, "%.1f mm");
                if (changed)
                {
                    UpdatePlotGeometry();
                    if (m_autoCalculate) m_needsRecalculation = true;
                }
                ImGui::PopID();
            }
        }
    }

    // 3. Propriedades dos Materiais
    if (ImGui::CollapsingHeader("3. Propriedades dos Materiais (NBR 6118)"))
    {
        bool changed = false;
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "Concreto Armado:");
        changed |= ImGui::SliderFloat("fck (MPa)", &m_fck, 15.0f, 90.0f, "%.0f MPa");

        ImGui::TextDisabled("Classes de Concreto:");
        float classes[] = { 20.0f, 25.0f, 30.0f, 35.0f, 40.0f, 50.0f, 60.0f };
        for (float c : classes)
        {
            char btnLabel[16];
            std::snprintf(btnLabel, sizeof(btnLabel), "C%.0f", c);
            if (ImGui::Button(btnLabel))
            {
                m_fck = c;
                changed = true;
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();

        changed |= ImGui::SliderFloat("γc (Concreto)", &m_gammaC, 1.0f, 2.0f, "%.2f");
        const char* cModels[] = { "ABNT NBR 6118:2014", "ABNT NBR 6118:2023" };
        changed |= ImGui::Combo("Modelo Concreto", &m_concreteModel, cModels, 2);

        if (ImGui::TreeNode("Pré-visualização Gráfico Concreto (σc x εc)"))
        {
            RenderConcreteConstitutivePlot(-1.0f, 180.0f);
            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "Aço Passivo:");
        changed |= ImGui::SliderFloat("fyk (MPa)", &m_fyk, 250.0f, 600.0f, "%.0f MPa");
        changed |= ImGui::SliderFloat("γs (Aço)", &m_gammaS, 1.0f, 2.0f, "%.2f");
        changed |= ImGui::SliderFloat("Es (GPa)", &m_Es, 150.0f, 250.0f, "%.0f GPa");

        if (ImGui::TreeNode("Pré-visualização Gráfico Aço (σs x εs)"))
        {
            RenderSteelConstitutivePlot(-1.0f, 180.0f);
            ImGui::TreePop();
        }

        if (changed && m_autoCalculate) m_needsRecalculation = true;
    }

    // 4. Esforços Solicitantes
    if (ImGui::CollapsingHeader("4. Esforços Solicitantes de Projeto", ImGuiTreeNodeFlags_DefaultOpen))
    {
        bool changed = false;
        changed |= ImGui::DragFloat("Força Normal Nsd (kN)", &m_Nsd, 5.0f, -5000.0f, 5000.0f, "%.1f kN");
        ImGui::TextDisabled("(+ Tração, - Compressão)");
        changed |= ImGui::DragFloat("Momento Msdx (kN.m)", &m_Msdx, 1.0f, -2000.0f, 2000.0f, "%.1f kN.m");
        changed |= ImGui::DragFloat("Momento Msdy (kN.m)", &m_Msdy, 1.0f, -2000.0f, 2000.0f, "%.1f kN.m");

        ImGui::Spacing();
        ImGui::TextDisabled("Casos de Carga Rápidos:");
        if (ImGui::Button("Flexão Pura")) ApplyLoadingPreset(0);
        ImGui::SameLine();
        if (ImGui::Button("Flexão Biaxial")) ApplyLoadingPreset(1);
        ImGui::SameLine();
        if (ImGui::Button("Compressão")) ApplyLoadingPreset(2);

        if (changed && m_autoCalculate) m_needsRecalculation = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("Status: %s", m_statusMessage.c_str());
    if (m_isRunning)
    {
        ImGui::ProgressBar(m_progress, ImVec2(-1, 18));
    }
    if (m_lastExecutionTime > 0.0)
    {
        ImGui::TextDisabled("Tempo IPC Worker: %.2f ms", m_lastExecutionTime);
    }

    ImGui::End();
}

void AppUI::RenderPlotPanel()
{
    float screenWidth = ImGui::GetIO().DisplaySize.x;
    float screenHeight = ImGui::GetIO().DisplaySize.y;
    float headerHeight = 46.0f;
    float leftPanelWidth = std::clamp(screenWidth * 0.32f, 360.0f, 460.0f);
    float plotPanelWidth = screenWidth - leftPanelWidth;

    ImGui::SetNextWindowPos(ImVec2(leftPanelWidth, headerHeight));
    ImGui::SetNextWindowSize(ImVec2(plotPanelWidth, screenHeight - headerHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("RightPlotWorkspace", nullptr, flags);

    if (m_hasResults)
    {
        if (m_isSizingMode)
        {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.22f, 0.32f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.75f, 1.00f, 0.85f));
            ImGui::BeginChild("SizingBanner", ImVec2(-1, 62.0f), true);
            ImGui::TextColored(ImVec4(0.20f, 0.85f, 1.00f, 1.00f),
                "🎯 DIMENSIONAMENTO DE ARMADURA CONCLUÍDO: As,req = %.2f cm² (ω = %.3f)",
                m_calcRequiredSteelArea, m_calcOmegaMechanicalRatio);
            ImGui::TextDisabled("Parâmetros Reduzidos (Ábacos): ν = %.3f | μx = %.3f | μy = %.3f | Taxa ρ = %.2f%% | %d Iterações",
                m_calcNuReducedNormal, m_calcMuXReducedMoment, m_calcMuYReducedMoment, m_calcSteelRatioPercent, m_calcSizingIterations);
            ImGui::EndChild();
            ImGui::PopStyleColor(2);
        }
        else if (m_isSafe)
        {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.24f, 0.14f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.82f, 0.45f, 0.80f));
            ImGui::BeginChild("SafetyBanner", ImVec2(-1, 52.0f), true);
            ImGui::TextColored(ImVec4(0.20f, 1.00f, 0.50f, 1.00f), "✔ SEÇÃO SEGURA — RESISTE AOS ESFORÇOS SOLICITANTES");
            ImGui::TextDisabled("Msd = (%.1f, %.1f) kN.m | Nsd = %.1f kN | fcd = %.2f MPa | fyd = %.2f MPa",
                m_Msdx, m_Msdy, m_Nsd, m_calcFcd, m_calcFyd);
            ImGui::EndChild();
            ImGui::PopStyleColor(2);
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.28f, 0.08f, 0.08f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.95f, 0.26f, 0.26f, 0.80f));
            ImGui::BeginChild("SafetyBanner", ImVec2(-1, 52.0f), true);
            ImGui::TextColored(ImVec4(1.00f, 0.40f, 0.40f, 1.00f), "✖ SEÇÃO INSEGURA — NÃO RESISTE! SOLICITAÇÃO FORA DA ENVOLTÓRIA");
            ImGui::TextDisabled("Msd = (%.1f, %.1f) kN.m | Nsd = %.1f kN | Aumente a seção ou armadura.",
                m_Msdx, m_Msdy, m_Nsd);
            ImGui::EndChild();
            ImGui::PopStyleColor(2);
        }
        ImGui::Spacing();
    }

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
        // ABA 3: Relações Constitutivas (Concreto & Aço)
        // -------------------------------------------------------------
        if (ImGui::BeginTabItem("Relações Constitutivas (Concreto & Aço)"))
        {
            RenderConstitutiveTab();
            ImGui::EndTabItem();
        }

        // -------------------------------------------------------------
        // ABA 4: Relatório Numérico e Propriedades da Seção
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

void AppUI::RenderConcreteConstitutivePlot(float plotWidth, float plotHeight)
{
    ConcreteProperties concrete;
    StressStrainConcreteModelType modelType = (m_concreteModel == 0) ?
        StressStrainConcreteModelType::PARABOLA_RECTANGLE_NBR6118_2014 :
        StressStrainConcreteModelType::PARABOLA_RECTANGLE_NBR6118_2023;

    concrete.setParameters(modelType, static_cast<double>(m_fck), static_cast<double>(m_gammaC));

    double fck = concrete.getFck();
    double fcd = concrete.getFcd();
    double ec2 = concrete.getStrainConcretePlastic();
    double ecu = concrete.getStrainConcreteRupture();
    double exponent = concrete.getStressStrainExponent();
    double factorMult = concrete.getFactorMultiplierFcd();
    double sigCMaxDesign = factorMult * fcd;

    int numPoints = 120;
    float maxStrainPlot = static_cast<float>(ecu * 1.15);

    std::vector<float> strainDesign;
    std::vector<float> stressDesign;
    std::vector<float> strainChar;
    std::vector<float> stressChar;

    for (int i = 0; i <= numPoints; ++i)
    {
        float eps = (maxStrainPlot * i) / numPoints;
        strainDesign.push_back(eps);

        if (eps <= ecu)
        {
            double s = concrete.computeStress(-eps);
            stressDesign.push_back(static_cast<float>(s));
        }
        else
        {
            stressDesign.push_back(0.0f);
        }

        strainChar.push_back(eps);
        if (eps <= ec2)
        {
            double s = fck * (1.0 - std::pow(1.0 - (eps / ec2), exponent));
            stressChar.push_back(static_cast<float>(s));
        }
        else if (eps <= ecu)
        {
            stressChar.push_back(static_cast<float>(fck));
        }
        else
        {
            stressChar.push_back(0.0f);
        }
    }

    float ec2MarkerX = static_cast<float>(ec2);
    float ec2MarkerY = static_cast<float>(sigCMaxDesign);
    float ecuMarkerX = static_cast<float>(ecu);
    float ecuMarkerY = static_cast<float>(sigCMaxDesign);

    float markerX[2] = { ec2MarkerX, ecuMarkerX };
    float markerY[2] = { ec2MarkerY, ecuMarkerY };

    if (ImPlot::BeginPlot("Diagrama Parábola-Retângulo do Concreto", ImVec2(plotWidth, plotHeight)))
    {
        ImPlot::SetupAxes("Deformação de Compressão εc (‰)", "Tensão no Concreto σc (MPa)");

        // Curva Característica (fck)
        ImPlot::SetNextLineStyle(ImVec4(0.5f, 0.5f, 0.9f, 0.7f), 1.5f);
        ImPlot::PlotLine("Curva Característica σck", strainChar.data(), stressChar.data(), static_cast<int>(strainChar.size()));

        // Curva de Cálculo (σcd)
        ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.75f, 1.0f, 1.0f), 2.5f);
        ImPlot::PlotLine("Curva de Cálculo σcd", strainDesign.data(), stressDesign.data(), static_cast<int>(strainDesign.size()));

        // Marcadores de Pontos Notáveis
        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 7.0f, ImVec4(1.0f, 0.3f, 0.2f, 1.0f), 2.0f);
        ImPlot::PlotScatter("Pontos Notáveis (εc2, εcu)", markerX, markerY, 2);

        // Linhas de Referência
        float refEps2X[2] = { ec2MarkerX, ec2MarkerX };
        float refEps2Y[2] = { 0.0f, static_cast<float>(fck * 1.05) };
        ImPlot::SetNextLineStyle(ImVec4(0.7f, 0.7f, 0.7f, 0.35f), 1.0f);
        ImPlot::PlotLine("##ref_ec2", refEps2X, refEps2Y, 2);

        float refEpsUX[2] = { ecuMarkerX, ecuMarkerX };
        float refEpsUY[2] = { 0.0f, static_cast<float>(fck * 1.05) };
        ImPlot::SetNextLineStyle(ImVec4(0.7f, 0.7f, 0.7f, 0.35f), 1.0f);
        ImPlot::PlotLine("##ref_ecu", refEpsUX, refEpsUY, 2);

        float refSigMaxX[2] = { 0.0f, maxStrainPlot };
        float refSigMaxY[2] = { ec2MarkerY, ec2MarkerY };
        ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.75f, 1.0f, 0.35f), 1.0f);
        ImPlot::PlotLine("##ref_sigmax", refSigMaxX, refSigMaxY, 2);

        ImPlot::EndPlot();
    }
}

void AppUI::RenderSteelConstitutivePlot(float plotWidth, float plotHeight)
{
    SteelProperties steel;
    steel.setParameters(StressStrainSteelModelType::PASSIVE_REINFORCEMENT, static_cast<double>(m_fyk), static_cast<double>(m_gammaS), static_cast<double>(m_Es));

    double fyk = steel.getFyk();
    double fyd = steel.getFyd();
    double Es = steel.getE(); // GPa
    double esy = steel.getStrainSteelYield(); // ‰
    double esu = steel.getStrainSteelRupture(); // ‰ (10)
    double esk = fyk / Es; // ‰

    int numPoints = 140;
    float maxStrain = static_cast<float>(esu * 1.2);

    std::vector<float> strainDesign;
    std::vector<float> stressDesign;
    std::vector<float> strainChar;
    std::vector<float> stressChar;

    for (int i = 0; i <= numPoints; ++i)
    {
        float eps = -maxStrain + (2.0f * maxStrain * i) / numPoints;

        strainDesign.push_back(eps);
        stressDesign.push_back(static_cast<float>(steel.computeStress(eps)));

        strainChar.push_back(eps);
        if (eps < -esu || eps > esu)
        {
            stressChar.push_back(0.0f);
        }
        else if (eps <= -esk)
        {
            stressChar.push_back(static_cast<float>(-fyk));
        }
        else if (eps >= esk)
        {
            stressChar.push_back(static_cast<float>(fyk));
        }
        else
        {
            stressChar.push_back(static_cast<float>(Es * eps));
        }
    }

    float markerX[4] = {
        static_cast<float>(-esu),
        static_cast<float>(-esy),
        static_cast<float>(esy),
        static_cast<float>(esu)
    };
    float markerY[4] = {
        static_cast<float>(-fyd),
        static_cast<float>(-fyd),
        static_cast<float>(fyd),
        static_cast<float>(fyd)
    };

    if (ImPlot::BeginPlot("Diagrama Tensão-Deformação do Aço", ImVec2(plotWidth, plotHeight)))
    {
        ImPlot::SetupAxes("Deformação do Aço εs (‰)", "Tensão no Aço σs (MPa)");

        // Curva Característica (fyk)
        ImPlot::SetNextLineStyle(ImVec4(0.9f, 0.6f, 0.2f, 0.6f), 1.5f);
        ImPlot::PlotLine("Curva Característica σsk", strainChar.data(), stressChar.data(), static_cast<int>(strainChar.size()));

        // Curva de Cálculo (fyd)
        ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.4f, 0.1f, 1.0f), 2.5f);
        ImPlot::PlotLine("Curva de Cálculo σsd", strainDesign.data(), stressDesign.data(), static_cast<int>(strainDesign.size()));

        // Marcadores de Escoamento e Ruptura
        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 7.0f, ImVec4(0.2f, 0.9f, 0.3f, 1.0f), 2.0f);
        ImPlot::PlotScatter("Escoamento / Ruptura (εsy, εsu)", markerX, markerY, 4);

        // Linhas de Referência
        float refFydX[2] = { -maxStrain, maxStrain };
        float refFydYPos[2] = { static_cast<float>(fyd), static_cast<float>(fyd) };
        float refFydYNeg[2] = { static_cast<float>(-fyd), static_cast<float>(-fyd) };

        ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.4f, 0.1f, 0.35f), 1.0f);
        ImPlot::PlotLine("##ref_fyd_pos", refFydX, refFydYPos, 2);
        ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.4f, 0.1f, 0.35f), 1.0f);
        ImPlot::PlotLine("##ref_fyd_neg", refFydX, refFydYNeg, 2);

        float refEsyXPos[2] = { static_cast<float>(esy), static_cast<float>(esy) };
        float refEsyXNeg[2] = { static_cast<float>(-esy), static_cast<float>(-esy) };
        float refEsyY[2] = { static_cast<float>(-fyk * 1.1), static_cast<float>(fyk * 1.1) };

        ImPlot::SetNextLineStyle(ImVec4(0.7f, 0.7f, 0.7f, 0.35f), 1.0f);
        ImPlot::PlotLine("##ref_esy_pos", refEsyXPos, refEsyY, 2);
        ImPlot::SetNextLineStyle(ImVec4(0.7f, 0.7f, 0.7f, 0.35f), 1.0f);
        ImPlot::PlotLine("##ref_esy_neg", refEsyXNeg, refEsyY, 2);

        ImPlot::EndPlot();
    }
}

void AppUI::RenderConstitutiveTab()
{
    if (ImGui::BeginTabBar("ConstitutiveSubTabs"))
    {
        if (ImGui::BeginTabItem("Concreto Armado (NBR 6118)"))
        {
            ConcreteProperties concrete;
            StressStrainConcreteModelType modelType = (m_concreteModel == 0) ?
                StressStrainConcreteModelType::PARABOLA_RECTANGLE_NBR6118_2014 :
                StressStrainConcreteModelType::PARABOLA_RECTANGLE_NBR6118_2023;

            concrete.setParameters(modelType, static_cast<double>(m_fck), static_cast<double>(m_gammaC));

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "Parâmetros Constitutivos do Concreto");
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::BeginTable("ConcreteParamsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Parâmetro");
                ImGui::TableSetupColumn("Símbolo");
                ImGui::TableSetupColumn("Valor");
                ImGui::TableSetupColumn("Descrição / Norma");
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Modelo Normativo");
                ImGui::TableSetColumnIndex(1); ImGui::Text("Norma");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%s", m_concreteModel == 0 ? "NBR 6118:2014" : "NBR 6118:2023");
                ImGui::TableSetColumnIndex(3); ImGui::Text("Diagrama Parábola-Retângulo");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Resistência Característica");
                ImGui::TableSetColumnIndex(1); ImGui::Text("fck");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.1f MPa", concrete.getFck());
                ImGui::TableSetColumnIndex(3); ImGui::Text("Resistência característica à compressão");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Coeficiente de Minoracão");
                ImGui::TableSetColumnIndex(1); ImGui::Text("γc");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f", concrete.getGammaC());
                ImGui::TableSetColumnIndex(3); ImGui::Text("Coeficiente parcial de segurança do concreto");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Resistência de Cálculo");
                ImGui::TableSetColumnIndex(1); ImGui::Text("fcd");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f MPa", concrete.getFcd());
                ImGui::TableSetColumnIndex(3); ImGui::Text("fcd = fck / γc");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Fator de Redução");
                ImGui::TableSetColumnIndex(1); ImGui::Text("η (eta)");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.4f", concrete.getStrenghtReductionFactor());
                ImGui::TableSetColumnIndex(3); ImGui::Text("η = 1.0 (fck<=40) ou (40/fck)^(1/3) (fck>40)");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Tensão de Pico de Cálculo");
                ImGui::TableSetColumnIndex(1); ImGui::Text("σc,max");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f MPa", concrete.getFactorMultiplierFcd() * concrete.getFcd());
                ImGui::TableSetColumnIndex(3); ImGui::Text("0.85 * fcd (2014) ou 0.85 * η * fcd (2023)");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Deformação Plástica Inicial");
                ImGui::TableSetColumnIndex(1); ImGui::Text("εc2");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.3f ‰", concrete.getStrainConcretePlastic());
                ImGui::TableSetColumnIndex(3); ImGui::Text("Início do patamar plástico de compressão");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Deformação de Ruptura");
                ImGui::TableSetColumnIndex(1); ImGui::Text("εcu");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.3f ‰", concrete.getStrainConcreteRupture());
                ImGui::TableSetColumnIndex(3); ImGui::Text("Limite de encurtamento do concreto");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Expoente da Parábola");
                ImGui::TableSetColumnIndex(1); ImGui::Text("n");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f", concrete.getStressStrainExponent());
                ImGui::TableSetColumnIndex(3); ImGui::Text("n = 2.0 (fck<=50) ou função de fck (fck>50)");

                ImGui::EndTable();
            }

            ImGui::Spacing();
            RenderConcreteConstitutivePlot(-1.0f, 380.0f);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Aço Passivo (Armaduras)"))
        {
            SteelProperties steel;
            steel.setParameters(StressStrainSteelModelType::PASSIVE_REINFORCEMENT, static_cast<double>(m_fyk), static_cast<double>(m_gammaS), static_cast<double>(m_Es));

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "Parâmetros Constitutivos do Aço");
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::BeginTable("SteelParamsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Parâmetro");
                ImGui::TableSetupColumn("Símbolo");
                ImGui::TableSetupColumn("Valor");
                ImGui::TableSetupColumn("Descrição / Norma");
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Resistência Característica ao Escoamento");
                ImGui::TableSetColumnIndex(1); ImGui::Text("fyk");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.1f MPa", steel.getFyk());
                ImGui::TableSetColumnIndex(3); ImGui::Text("Tensão de escoamento característica");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Coeficiente de Minoracão");
                ImGui::TableSetColumnIndex(1); ImGui::Text("γs");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f", steel.getGammaS());
                ImGui::TableSetColumnIndex(3); ImGui::Text("Coeficiente parcial de segurança do aço");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Resistência de Escoamento de Cálculo");
                ImGui::TableSetColumnIndex(1); ImGui::Text("fyd");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f MPa", steel.getFyd());
                ImGui::TableSetColumnIndex(3); ImGui::Text("fyd = fyk / γs");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Módulo de Elasticidade");
                ImGui::TableSetColumnIndex(1); ImGui::Text("Es");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.1f GPa (%.0f MPa)", steel.getE(), steel.getE() * 1000.0);
                ImGui::TableSetColumnIndex(3); ImGui::Text("Rigidez elástica das barras de aço");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Deformação de Escoamento de Cálculo");
                ImGui::TableSetColumnIndex(1); ImGui::Text("εsy");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.3f ‰", steel.getStrainSteelYield());
                ImGui::TableSetColumnIndex(3); ImGui::Text("εsy = fyd / Es");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Deformação Limite de Ruptura");
                ImGui::TableSetColumnIndex(1); ImGui::Text("εsu");
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.1f ‰ (1.0%%)", steel.getStrainSteelRupture());
                ImGui::TableSetColumnIndex(3); ImGui::Text("Limite normativo de deformação do aço");

                ImGui::EndTable();
            }

            ImGui::Spacing();
            RenderSteelConstitutivePlot(-1.0f, 380.0f);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Visão Comparativa Lado a Lado"))
        {
            ImGui::Spacing();
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float plotHeight = avail.y - 40.0f;
            if (plotHeight < 300.0f) plotHeight = 300.0f;

            if (ImGui::BeginTable("SideBySideTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "Concreto Armado (σc x εc)");
                RenderConcreteConstitutivePlot(-1.0f, plotHeight);

                ImGui::TableSetColumnIndex(1);
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), "Aço Passivo (σs x εs)");
                RenderSteelConstitutivePlot(-1.0f, plotHeight);

                ImGui::EndTable();
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}


