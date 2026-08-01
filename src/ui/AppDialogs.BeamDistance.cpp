#include "ui/AppDialogs.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include "imgui.h"

namespace
{
    Point2D ComputeBeamDistanceCreatedNodeWorld(const EditorState& state, const std::string& beamDistanceInput)
    {
        double distance = std::atof(beamDistanceInput.c_str());

        if (state.beamTool.beamDistanceMode == BeamToolState::DistanceInputMode::BeamAlongSegment)
        {
            const Point2D delta = Point2D{
                state.beamTool.beamDistanceSegmentEndWorld.x - state.beamTool.beamDistanceSegmentStartWorld.x,
                state.beamTool.beamDistanceSegmentEndWorld.y - state.beamTool.beamDistanceSegmentStartWorld.y};
            const double length = sqrt(delta.x * delta.x + delta.y * delta.y);
            if (length <= 1.0e-9)
            {
                return state.beamTool.beamDistanceSegmentStartWorld;
            }

            distance = std::clamp(distance, 0.0, length);

            const double normalizedDistance = distance / length;
            return Point2D{
                state.beamTool.beamDistanceSegmentStartWorld.x + delta.x * normalizedDistance,
                state.beamTool.beamDistanceSegmentStartWorld.y + delta.y * normalizedDistance};
        }

        if (state.beamTool.beamDistanceLocksX)
        {
            if (state.beamTool.beamDistanceGuideWorld.y - state.beamTool.beamDistanceClickWorld.y > 0.0f)
            {
                distance = -distance;
            }

            return Point2D{
                state.beamTool.beamDistanceGuideWorld.x,
                state.beamTool.beamDistanceGuideWorld.y + distance};
        }

        if (state.beamTool.beamDistanceGuideWorld.x - state.beamTool.beamDistanceClickWorld.x > 0.0f)
        {
            distance = -distance;
        }

        return Point2D{
            state.beamTool.beamDistanceGuideWorld.x + distance,
            state.beamTool.beamDistanceGuideWorld.y};
    }

    void SetBeamDistanceDialogPosition(const EditorState& state)
    {
        if (state.beamTool.beamDistanceMode != BeamToolState::DistanceInputMode::BeamAlongSegment)
        {
            ImGui::SetNextWindowPos(
                ImVec2(
                    state.beamTool.beamDistanceScreenPosition.x - 90.0f,
                    state.beamTool.beamDistanceScreenPosition.y - 72.0f),
                ImGuiCond_Appearing);
            return;
        }

        Vector2 segmentDirection = Vector2{
            static_cast<float>(state.beamTool.beamDistanceSegmentEndWorld.x - state.beamTool.beamDistanceSegmentStartWorld.x),
            static_cast<float>(state.beamTool.beamDistanceSegmentEndWorld.y - state.beamTool.beamDistanceSegmentStartWorld.y)};
        float segmentLength = sqrtf(
            segmentDirection.x * segmentDirection.x +
            segmentDirection.y * segmentDirection.y);
        if (segmentLength <= 1.0e-6f)
        {
            segmentLength = 1.0f;
        }

        segmentDirection.x /= segmentLength;
        segmentDirection.y /= segmentLength;

        Vector2 normal = Vector2{-segmentDirection.y, segmentDirection.x};
        if (normal.y > 0.0f || (std::abs(normal.y) <= 1.0e-6f && normal.x < 0.0f))
        {
            normal.x = -normal.x;
            normal.y = -normal.y;
        }

        const ImVec2 anchor = ImVec2(
            state.beamTool.beamDistanceScreenPosition.x + normal.x * 72.0f,
            state.beamTool.beamDistanceScreenPosition.y + normal.y * 72.0f);
        const ImVec2 pivot = ImVec2(
            0.5f,
            0.5f);

        ImGui::SetNextWindowPos(anchor, ImGuiCond_Appearing, pivot);
    }
}

BeamDistanceDialogResult AppDialogs::DrawBeamDistanceInputDialog(const EditorState& state)
{
    BeamDistanceDialogResult result{};

    if (!state.beamTool.isBeamDistanceInputOpen)
    {
        beamDistanceDialogWasOpen = false;
        beamDistanceInputShouldFocus = false;
        beamDistanceInput.clear();
        return result;
    }

    const bool justOpened = !beamDistanceDialogWasOpen;
    beamDistanceDialogWasOpen = true;

    if (justOpened)
    {
        const double defaultDistance =
            state.beamTool.beamDistanceMode == BeamToolState::DistanceInputMode::BeamAlongSegment
                ? sqrt(
                    static_cast<double>(state.beamTool.beamDistanceClickWorld.x - state.beamTool.beamDistanceSegmentStartWorld.x) *
                        static_cast<double>(state.beamTool.beamDistanceClickWorld.x - state.beamTool.beamDistanceSegmentStartWorld.x) +
                    static_cast<double>(state.beamTool.beamDistanceClickWorld.y - state.beamTool.beamDistanceSegmentStartWorld.y) *
                        static_cast<double>(state.beamTool.beamDistanceClickWorld.y - state.beamTool.beamDistanceSegmentStartWorld.y))
                : (state.beamTool.beamDistanceLocksX
                    ? std::abs(state.beamTool.beamDistanceClickWorld.y - state.beamTool.beamDistanceGuideWorld.y)
                    : std::abs(state.beamTool.beamDistanceClickWorld.x - state.beamTool.beamDistanceGuideWorld.x));

        char defaultBuf[64];
        std::snprintf(defaultBuf, sizeof(defaultBuf), "%.6g", defaultDistance);
        beamDistanceInput = defaultBuf;
        beamDistanceInputShouldFocus = true;
    }

    result.previewNodeWorld = ComputeBeamDistanceCreatedNodeWorld(state, beamDistanceInput);
    result.hasPreviewNode = true;

    SetBeamDistanceDialogPosition(state);
    ImGui::SetNextWindowBgAlpha(0.96f);

    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;

    bool keepOpen = true;
    if (ImGui::Begin("Distância###BeamDistanceInput", &keepOpen, flags))
    {
        ImGui::TextUnformatted(
            state.beamTool.beamDistanceMode == BeamToolState::DistanceInputMode::BeamAlongSegment
                ? "Distância do início da barra:"
                : "Distância do nó selecionado:");

        if (beamDistanceInputShouldFocus)
        {
            ImGui::SetKeyboardFocusHere();
            beamDistanceInputShouldFocus = false;
        }

        char textBuf[64];
        std::snprintf(textBuf, sizeof(textBuf), "%s", beamDistanceInput.c_str());
        ImGui::SetNextItemWidth(180.0f);
        if (ImGui::InputText(
            "##BeamDistanceValue",
            textBuf,
            sizeof(textBuf),
            ImGuiInputTextFlags_CharsNoBlank))
        {
            beamDistanceInput = textBuf;
        }

        result.previewNodeWorld = ComputeBeamDistanceCreatedNodeWorld(state, beamDistanceInput);
        result.hasPreviewNode = true;

        const float buttonWidth =
            (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

        if (ImGui::Button("Confirmar", ImVec2(buttonWidth, 0.0f)))
        {
            result.action = BeamDistanceDialogResult::Action::Confirm;
            result.createdNodeWorld = ComputeBeamDistanceCreatedNodeWorld(state, beamDistanceInput);
            result.shouldCloseDialog = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancelar", ImVec2(buttonWidth, 0.0f)))
        {
            result.action = BeamDistanceDialogResult::Action::Cancel;
            result.shouldCloseDialog = true;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Enter) ||
            ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) ||
            ImGui::IsKeyPressed(ImGuiKey_Space))
        {
            result.action = BeamDistanceDialogResult::Action::Confirm;
            result.createdNodeWorld = ComputeBeamDistanceCreatedNodeWorld(state, beamDistanceInput);
            result.shouldCloseDialog = true;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            result.action = BeamDistanceDialogResult::Action::Cancel;
            result.shouldCloseDialog = true;
        }
    }

    ImGui::End();

    if (!keepOpen)
    {
        result.action = BeamDistanceDialogResult::Action::Cancel;
        result.shouldCloseDialog = true;
    }

    return result;
}
