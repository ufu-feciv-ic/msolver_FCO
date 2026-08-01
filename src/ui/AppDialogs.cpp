#include "ui/AppDialogs.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <initializer_list>

#include "imgui.h"
#include "ui/ToolbarDialogRequest.h"

namespace
{
    constexpr const char* materialDialogPopupId = u8"Material###MaterialDialog";
    constexpr const char* sectionDialogPopupId  = u8"Seção###SectionDialog";
    constexpr const char* progressDialogPopupId = u8"Processando...###ProgressDialog";
}

void AppDialogs::ApplyRequest(ToolbarDialogRequest request, const ProjectDocument& document, const EditorState& state)
{
    switch (request)
    {
    case ToolbarDialogRequest::OpenCreateMaterial:
        BeginCreateMaterial(document);
        closeMaterialDialogRequested = false;
        ImGui::OpenPopup(materialDialogPopupId);
        break;

    case ToolbarDialogRequest::OpenEditMaterial:
        BeginEditMaterial(document, state);
        if (materialDialogMode != DialogMode::None)
        {
            closeMaterialDialogRequested = false;
            ImGui::OpenPopup(materialDialogPopupId);
        }
        break;

    case ToolbarDialogRequest::OpenCreateSection:
        BeginCreateSection(document);
        closeSectionDialogRequested = false;
        ImGui::OpenPopup(sectionDialogPopupId);
        break;

    case ToolbarDialogRequest::OpenEditSection:
        BeginEditSection(document, state);
        if (sectionDialogMode != DialogMode::None)
        {
            closeSectionDialogRequested = false;
            ImGui::OpenPopup(sectionDialogPopupId);
        }
        break;

    case ToolbarDialogRequest::None:
    default:
        break;
    }
}

DialogFrameResult AppDialogs::Draw(const ProjectDocument& document, const EditorState& state, const AnalysisState& analysisState)
{
    DialogFrameResult result{};
    result.document.material = DrawMaterialDialog(document, state);
    result.document.section = DrawSectionDialog(document, state);
    result.beamDistance = DrawBeamDistanceInputDialog(state);
    
    result.analysis = DrawProgressDialog(analysisState);

    return result;
}

bool AppDialogs::HandleEscape()
{
    const bool materialPopupOpen = ImGui::IsPopupOpen(materialDialogPopupId);
    const bool sectionPopupOpen  = ImGui::IsPopupOpen(sectionDialogPopupId);

    if (materialDialogMode != DialogMode::None || materialPopupOpen)
    {
        closeMaterialDialogRequested = true;
        return true;
    }

    if (sectionDialogMode != DialogMode::None || sectionPopupOpen)
    {
        closeSectionDialogRequested = true;
        return true;
    }

    return false;
}

float AppDialogs::ComputeMaxTextWidth(std::initializer_list<const char*> labels) const
{
    float width = 0.0f;

    for (const char* label : labels)
    {
        width = std::max(width, ImGui::CalcTextSize(label).x);
    }

    return width;
}

float AppDialogs::ComputeDialogWidth(float labelWidth, float inputWidth, float unitWidth) const
{
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    return dialogWindowPaddingX * 2.0f + labelWidth + spacing + inputWidth + spacing + unitWidth;
}

bool AppDialogs::BeginPropertyTable(const char* id, float labelWidth, float inputWidth, float unitWidth) const
{
    if (!ImGui::BeginTable(id, 3, ImGuiTableFlags_SizingFixedFit))
    {
        return false;
    }

    ImGui::TableSetupColumn("Rótulo", ImGuiTableColumnFlags_WidthFixed, labelWidth);
    ImGui::TableSetupColumn("Valor", ImGuiTableColumnFlags_WidthFixed, inputWidth);
    ImGui::TableSetupColumn("Unidade", ImGuiTableColumnFlags_WidthFixed, unitWidth);

    return true;
}

void AppDialogs::DrawDoubleInputRow(
    const char* label,
    const char* inputId,
    double& value,
    const char* unitLabel,
    const char* format) const
{
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputDouble(inputId, &value, 0.0, 0.0, format);

    ImGui::TableSetColumnIndex(2);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(unitLabel);
}

void AppDialogs::DrawPositiveDoubleInputRow(
    const char* label,
    const char* inputId,
    double& value,
    double& lastValidValue,
    const char* unitLabel,
    const char* format) const
{
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputDouble(inputId, &value, 0.0, 0.0, format);

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        if (value > 0.0)
        {
            lastValidValue = value;
        }
        else
        {
            value = lastValidValue;
        }
    }
    else if (!ImGui::IsItemActive() && value > 0.0)
    {
        lastValidValue = value;
    }

    ImGui::TableSetColumnIndex(2);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(unitLabel);
}

AnalysisDialogResult AppDialogs::DrawProgressDialog(const AnalysisState& analysisState) const
{
    if (!analysisState.showProgressDialog)
    {
        analysisPopupWasVisible = false;
        analysisPopupDisplayProgress = 0.0f;
        return {AnalysisDialogResult::Action::None};
    }

    AnalysisDialogResult result{AnalysisDialogResult::Action::None};
    const char* windowId = u8"Processamento de Análise###AnalysisProgressOverlay";

    if (!analysisPopupWasVisible)
    {
        analysisPopupWasVisible = true;
        analysisPopupDisplayProgress = analysisState.progressValue;
    }

    const float deltaTime = ImGui::GetIO().DeltaTime;
    const float smoothing = 0.000001f;
    analysisPopupDisplayProgress =
        (analysisPopupDisplayProgress - analysisState.progressValue) * powf(smoothing, deltaTime) +
        analysisState.progressValue;
    analysisPopupDisplayProgress = std::clamp(analysisPopupDisplayProgress, 0.0f, 1.0f);

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(460.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.95f);

    // Flags para janela não bloqueante, sem título e fixa
    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | 
                            ImGuiWindowFlags_NoMove | 
                            ImGuiWindowFlags_NoSavedSettings | 
                            ImGuiWindowFlags_NoCollapse | 
                            ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoFocusOnAppearing;

    if (ImGui::Begin(windowId, nullptr, flags))
    {
        const char* titleText = "";
        if (analysisState.options.type == AnalysisType::Buckling) {
            titleText = analysisState.isRunning ? u8"Calculando Modos de Flambagem" : u8"Flambagem Concluída";
        } else {
            titleText = analysisState.isRunning ? u8"Processando Análise Estrutural" : u8"Análise Concluída";
        }
        
        if (analysisState.workerFailed) {
            titleText = u8"Falha na Análise";
        }

        const char* statusText = analysisState.isRunning ? analysisState.progressLabel.c_str() : (analysisState.workerFailed ? u8"Ocorreu um erro no processo." : u8"Resultados prontos para visualização.");

        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(244, 247, 250, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(210, 216, 224, 255));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        if (ImGui::BeginChild("##AnalysisPopupHeader", ImVec2(0.0f, 76.0f), true, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(40, 48, 60, 255));
            ImGui::TextUnformatted(titleText);
            ImGui::PopStyleColor();
            ImGui::Spacing();
            
            if (analysisState.workerFailed) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 50, 50, 255));
            } else if (!analysisState.isRunning) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(50, 150, 50, 255));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(65, 72, 85, 255));
            }
            ImGui::TextUnformatted(statusText);
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);

        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(74, 138, 230, 255));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(226, 232, 239, 255));
        ImGui::ProgressBar(analysisState.workerFailed ? 1.0f : analysisPopupDisplayProgress, ImVec2(-FLT_MIN, 18.0f), "");
        ImGui::PopStyleColor(2);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float buttonWidth = 140.0f;
        ImGui::BeginChild("##AnalysisPopupActions", ImVec2(0.0f, 32.0f), false, ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPosX(std::max(0.0f, (ImGui::GetContentRegionAvail().x - buttonWidth) * 0.5f));
        if (analysisState.isRunning)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(204, 94, 72, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(216, 108, 86, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(182, 78, 58, 255));
            if (ImGui::Button("Cancelar", ImVec2(buttonWidth, 0.0f)))
            {
                result.action = AnalysisDialogResult::Action::Abort;
            }
            ImGui::PopStyleColor(3);
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(88, 132, 196, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(102, 146, 210, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(72, 116, 180, 255));
            if (ImGui::Button("Fechar", ImVec2(buttonWidth, 0.0f)))
            {
                result.action = AnalysisDialogResult::Action::Close;
            }
            ImGui::PopStyleColor(3);
        }
        ImGui::EndChild();

        ImGui::End();
    }

    return result;
}

void AppDialogs::DrawDialogButtons() const
{
    const float totalWidth = dialogButtonWidth * 2.0f + ImGui::GetStyle().ItemSpacing.x;
    const float availableWidth = ImGui::GetContentRegionAvail().x;

    if (availableWidth > totalWidth)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availableWidth - totalWidth) * 0.5f);
    }
}


