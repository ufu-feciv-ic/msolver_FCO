#include "ui/LeftPanel.h"

#include "imgui.h"

namespace
{
    constexpr float topToolbarHeight = 149.0f;
    constexpr float leftPanelWidth = 73.0f;
}

void LeftPanel::Draw(EditorState& editorState)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(
        ImVec2(
            viewport->Pos.x,
            viewport->Pos.y + topToolbarHeight));

    ImGui::SetNextWindowSize(
        ImVec2(
            leftPanelWidth,
            viewport->Size.y - topToolbarHeight));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));

    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse;

    if (!ImGui::Begin("LeftPanel", nullptr, flags))
    {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    if (ImGui::BeginChild("AuxiliosSection", ImVec2(0.0f, 0.0f), false))
    {
        const char* title = "Auxílios";
        const float titleWidth = ImGui::CalcTextSize(title).x;
        const float titleOffsetX = (ImGui::GetContentRegionAvail().x - titleWidth) * 0.5f;
        if (titleOffsetX > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + titleOffsetX);
        }

        ImGui::TextUnformatted(title);
        ImGui::Separator();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 5.0f));
        ImGui::Checkbox("Grade", &editorState.view.showGrid);
        ImGui::Checkbox("Orto", &editorState.view.showGuides);
        ImGui::Checkbox("Snap", &editorState.view.snapToGrid);
        ImGui::PopStyleVar(2);
    }

    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleVar();
}
