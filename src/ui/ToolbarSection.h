#pragma once

#include <cstring>
#include <functional>
#include <initializer_list>

#include "imgui.h"

namespace ToolbarSection
{
    struct ButtonSpec
    {
        const char* label = "";
        bool enabled = true;
        std::function<void()> onClick;
        bool active = false;
    };

    constexpr float sectionPaddingX = 4.0f;
    constexpr float sectionPaddingY = 0.0f;
    constexpr float sectionContentInsetTopY = 2.0f;
    constexpr float sectionContentLiftY = 0.0f;
    constexpr float preservedLegacyTitleGapY = 12.0f;
    constexpr float contentToTitleGapY = 3.0f;
    constexpr float maxFullHeightButtonHeight = 93.0f;

    inline float GetTitleVerticalOffset(const char* title)
    {
        return std::strcmp(title, "Visualização") == 0 ? 2.0f : 3.0f;
    }

    inline float ComputeWidth(
        int buttonCount,
        float buttonWidth,
        float horizontalSpacing)
    {
        if (buttonCount <= 0)
        {
            return sectionPaddingX * 2.0f;
        }

        return sectionPaddingX * 2.0f +
               buttonCount * buttonWidth +
               (buttonCount - 1) * horizontalSpacing;
    }

    inline void BeginSection(const char* id, float width)
    {
        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            ImVec2(sectionPaddingX, sectionPaddingY));

        ImGui::PushStyleColor(
            ImGuiCol_Border,
            ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        ImGui::BeginChild(
            id,
            ImVec2(width, 0.0f),
            true,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        const float initialCursorY = ImGui::GetCursorPosY();
        ImGui::SetCursorPosY(std::max(
            0.0f,
            initialCursorY + sectionContentInsetTopY - sectionContentLiftY));
    }

    inline float GetButtonHeight(const char* title)
    {
        const float titleHeight = ImGui::CalcTextSize(title).y;
        const float titleVerticalOffset = GetTitleVerticalOffset(title);

        float height = ImGui::GetContentRegionAvail().y - titleHeight - contentToTitleGapY + titleVerticalOffset;
        if (height < 1.0f)
        {
            height = 1.0f;
        }

        return height;
    }

    inline float ClampFullHeightButtonHeight(float height)
    {
        return std::max(1.0f, std::min(height, maxFullHeightButtonHeight));
    }

    inline void PushTitleToBottom(const char* title)
    {
        const float titleHeight = ImGui::CalcTextSize(title).y;
        const float titleVerticalOffset = GetTitleVerticalOffset(title);
        const float fillerHeight =
            ImGui::GetContentRegionAvail().y - titleHeight + titleVerticalOffset;
        if (fillerHeight > 0.0f)
        {
            ImGui::Dummy(ImVec2(0.0f, fillerHeight));
        }
    }

    inline void EndSection(const char* title)
    {
        const float titleWidth = ImGui::CalcTextSize(title).x;
        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const float titleVerticalOffset = GetTitleVerticalOffset(title);

        float cursorX = (availableWidth - titleWidth) * 0.5f;
        if (cursorX > 0.0f)
        {
            ImGui::SetCursorPosX(cursorX);
        }

        const float titleYOffset = titleVerticalOffset - preservedLegacyTitleGapY;
        if (titleYOffset != 0.0f)
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + titleYOffset);
        }

        ImGui::TextUnformatted(title);
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    inline void VerticalSeparator()
    {
        ImGui::SameLine();

        const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
        ImVec2 p = ImGui::GetCursorScreenPos();
        float height = ImGui::GetContentRegionAvail().y;

        const float x = p.x - spacing.x * 0.5f;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddLine(
            ImVec2(x, p.y),
            ImVec2(x, p.y + height),
            IM_COL32(150, 150, 150, 255));

        ImGui::SameLine();
    }

    inline void PushActiveButtonStyle()
    {
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(116, 176, 255, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(136, 188, 255, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(92, 160, 246, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(36, 116, 255, 255));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    }

    inline void PopActiveButtonStyle()
    {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
    }

    inline bool DrawButton(const ButtonSpec& button, const ImVec2& size)
    {
        if (!button.enabled)
        {
            ImGui::BeginDisabled();
        }

        if (button.active)
        {
            PushActiveButtonStyle();
        }

        const bool pressed = ImGui::Button(button.label, size);

        if (button.active)
        {
            PopActiveButtonStyle();
        }

        if (!button.enabled)
        {
            ImGui::EndDisabled();
        }

        if (pressed && button.onClick)
        {
            button.onClick();
        }

        return pressed;
    }

    inline void DrawButtonSection(
        const char* title,
        std::initializer_list<ButtonSpec> buttons,
        float buttonWidth = 78.0f,
        float horizontalSpacing = 7.0f)
    {
        const float width = ComputeWidth(
            static_cast<int>(buttons.size()),
            buttonWidth,
            horizontalSpacing);

        BeginSection(title, width);

        const float buttonHeight = ClampFullHeightButtonHeight(GetButtonHeight(title));

        int index = 0;
        for (const ButtonSpec& button : buttons)
        {
            if (index > 0)
            {
                ImGui::SameLine(0.0f, horizontalSpacing);
            }

            DrawButton(button, ImVec2(buttonWidth, buttonHeight));

            ++index;
        }

        PushTitleToBottom(title);
        EndSection(title);
    }
}
