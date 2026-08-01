#include "ui/TopToolbar.h"

#include "app/AppVersion.h"
#include "rlImGui.h"
#include "editor/ops/PropertySelectionOperations.h"
#include "ui/ToolbarDialogRequest.h"
#include "ui/ToolbarSection.h"
#include "utils/UnitConversion.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "imgui.h"

namespace
{
    constexpr float toolbarHeight = 150.0f;
    constexpr float buttonWidth = 78.0f;
    constexpr float itemSpacingX = 7.0f;
    constexpr float dimensionControlVerticalOffset = 15.0f;
    constexpr float dimensionComboTargetHeight = 20.0f;
    constexpr ImVec2 dimensionControlItemSpacing = ImVec2(4.0f, 1.0f);
    constexpr ImVec2 visualizationFramePadding = ImVec2(4.0f, 3.0f);
    constexpr ImVec2 visualizationItemSpacing = ImVec2(4.0f, 5.0f);
    constexpr ImVec2 visualizationCellPadding = ImVec2(2.0f, 2.0f);
    constexpr float toolbarContentHeightGainY = 10.0f;
    constexpr float loadTabSectionTrailingTrimX = 6.0f;

    float ComputeComboPreviewWidth(std::initializer_list<const char*> previewValues, float minimumWidth = 0.0f)
    {
        float maxTextWidth = 0.0f;
        for (const char* text : previewValues)
        {
            maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(text).x);
        }

        const ImGuiStyle& style = ImGui::GetStyle();
        const float arrowWidth = ImGui::GetFrameHeight();
        const float comboWidth =
            maxTextWidth +
            style.FramePadding.x * 2.0f +
            style.ItemInnerSpacing.x +
            arrowWidth;
        return std::max(minimumWidth, comboWidth);
    }

    float ComputeCheckboxCellWidth(const char* label, ImVec2 framePadding)
    {
        const ImGuiStyle& style = ImGui::GetStyle();
        const float checkboxSize = ImGui::GetFontSize() + framePadding.y * 2.0f;
        return checkboxSize + style.ItemInnerSpacing.x + ImGui::CalcTextSize(label).x;
    }

    float ComputeDimensionSectionWidth(float buttonGroupWidth, float controlGroupWidth)
    {
        return ToolbarSection::sectionPaddingX * 2.0f +
               buttonGroupWidth +
               itemSpacingX +
               controlGroupWidth;
    }

    struct LoadInputFieldState
    {
        char text[32]{};
        bool isEditing = false;
    };

    struct LoadComponentSelectionState
    {
        bool hasSelection = false;
        bool isMixed = false;
        double displayValue = 0.0;
    };

    bool TryParseTextToDouble(const char* text, double& outValue)
    {
        if (text == nullptr)
        {
            return false;
        }

        while (*text != '\0' && std::isspace(static_cast<unsigned char>(*text)) != 0)
        {
            ++text;
        }

        if (*text == '\0')
        {
            return false;
        }

        char* end = nullptr;
        const double parsedValue = std::strtod(text, &end);
        if (end == text)
        {
            return false;
        }

        while (*end != '\0' && std::isspace(static_cast<unsigned char>(*end)) != 0)
        {
            ++end;
        }

        if (*end != '\0')
        {
            return false;
        }

        outValue = parsedValue;
        return true;
    }

    void SetLoadInputText(LoadInputFieldState& fieldState, bool isMixed, double displayValue)
    {
        if (fieldState.isEditing)
        {
            return;
        }

        if (isMixed)
        {
            fieldState.text[0] = '\0';
            return;
        }

        std::snprintf(fieldState.text, sizeof(fieldState.text), "%.6g", displayValue);
    }

    std::uint64_t ComputeBeamSelectionToken(const ProjectDocument& projectDocument, const Selection& selection)
    {
        std::vector<int> beamIds;
        beamIds.reserve(selection.beamIds.size());
        for (int beamId : selection.beamIds)
        {
            if (projectDocument.FindBeamById(beamId) != nullptr)
            {
                beamIds.push_back(beamId);
            }
        }

        std::sort(beamIds.begin(), beamIds.end());

        std::uint64_t hash = 1469598103934665603ull;
        for (int beamId : beamIds)
        {
            hash ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(beamId));
            hash *= 1099511628211ull;
        }

        hash ^= static_cast<std::uint64_t>(beamIds.size());
        hash *= 1099511628211ull;
        return hash;
    }
}

void TopToolbar::FillRequests(
    FrameRequests& requests,
    ProjectDocument& projectDocument,
    EditorState& editorState,
    const AnalysisState& analysisState,
    float cameraZoom)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, toolbarHeight));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));

    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar;

    if (!ImGui::Begin("TopToolbar", nullptr, flags))
    {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(itemSpacingX, 6.0f));

    if (ImGui::BeginTabBar("TopToolbarTabs"))
    {
        ImGuiTabItemFlags homeTabFlags = ImGuiTabItemFlags_None;
        if (editorState.uiRequestedHomeTab)
        {
            homeTabFlags |= ImGuiTabItemFlags_SetSelected;
            editorState.uiRequestedHomeTab = false;
        }

        if (ImGui::BeginTabItem("Página Inicial", nullptr, homeTabFlags))
        {
            DrawHomeTab(requests, projectDocument, editorState, cameraZoom);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Carregamentos"))
        {
            DrawLoadsTab(requests, projectDocument, editorState);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Materiais e Seções"))
        {
            DrawPropertiesTab(requests, projectDocument, editorState);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Discretização"))
        {
            DrawDiscretizationTab(requests, projectDocument, editorState);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Imperfeições"))
        {
            DrawImperfectionsTab(requests, projectDocument, editorState, analysisState);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Análise"))
        {
            DrawAnalysisTab(requests, projectDocument, editorState, analysisState);
            ImGui::EndTabItem();
        }

        if (analysisState.hasResults)
        {
            ImGuiTabItemFlags resultsTabFlags = ImGuiTabItemFlags_None;
            if (editorState.uiRequestedResultsTab)
            {
                resultsTabFlags |= ImGuiTabItemFlags_SetSelected;
                editorState.uiRequestedResultsTab = false;
            }

            if (ImGui::BeginTabItem("Resultados", nullptr, resultsTabFlags))
            {
                DrawResultsTab(requests, projectDocument, editorState, analysisState);
                ImGui::EndTabItem();
            }
        }

        if (ImGui::BeginTabItem("Sobre"))
        {
            DrawAboutTab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::PopStyleVar();
    ImGui::End();
    ImGui::PopStyleVar();

    return;
}

void TopToolbar::DrawHomeTab(
    FrameRequests& requests,
    ProjectDocument& projectDocument,
    EditorState& editorState,
    float cameraZoom)
{
    const EditorTool activeTool = editorState.activeTool;

    ToolbarSection::DrawButtonSection(
        "Nós",
        {
            {"Adicionar", true, [&]() { requests.editor = EditorRequest::ActivateAddNodeTool; }, activeTool == EditorTool::AddNode},
            {"Remover",   true, [&]() { requests.editor = EditorRequest::InvokeNodeRemoveAction; }, activeTool == EditorTool::RemoveNode},
        },
        buttonWidth,
        itemSpacingX);

    ToolbarSection::VerticalSeparator();

    ToolbarSection::DrawButtonSection(
        "Barras",
        {
            {"Adicionar", true, [&]() { requests.editor = EditorRequest::ActivateAddBeamTool; }, activeTool == EditorTool::AddBeam},
            {"Remover",   true, [&]() { requests.editor = EditorRequest::InvokeBeamRemoveAction; }, activeTool == EditorTool::RemoveBeam}
        },
        buttonWidth,
        itemSpacingX);

    ToolbarSection::VerticalSeparator();

    ToolbarSection::DrawButtonSection(
        "Editar",
        {
            {"Mover",    true,  [&]() { requests.editor = EditorRequest::ActivateMoveNodeTool; }, activeTool == EditorTool::MoveNode},
            {"Copiar",   true,  [&]() { requests.editor = EditorRequest::ActivateCopySelectionTool; }, activeTool == EditorTool::CopySelection},
            {"Espelhar", true,  [&]() { requests.editor = EditorRequest::ActivateMirrorSelectionTool; }, activeTool == EditorTool::MirrorSelection}
        },
        buttonWidth,
        itemSpacingX);

    ToolbarSection::VerticalSeparator();

    ToolbarSection::DrawButtonSection(
        "Apoios",
        {
            {"Nenhum",  true, [&]() { requests.editor = EditorRequest::SetSupportNone; }, activeTool == EditorTool::SetSupportNone},
            {"X",       true, [&]() { requests.editor = EditorRequest::SetSupportX; }, activeTool == EditorTool::SetSupportX},
            {"Y",       true, [&]() { requests.editor = EditorRequest::SetSupportY; }, activeTool == EditorTool::SetSupportY},
            {"XY",      true, [&]() { requests.editor = EditorRequest::SetSupportXY; }, activeTool == EditorTool::SetSupportXY},
            {"Engaste", true, [&]() { requests.editor = EditorRequest::SetSupportFixed; }, activeTool == EditorTool::SetSupportFixed}
        },
        buttonWidth,
        itemSpacingX);

    ToolbarSection::VerticalSeparator();

    ToolbarSection::DrawButtonSection(
        u8"Benchmarks",
        {
            {"Williams", true, [&]() { requests.editor = EditorRequest::InvokeInsertWilliamsBenchmark; }, false},
            {"Lee",      true, [&]() { requests.editor = EditorRequest::InvokeInsertLeeBenchmark; },      false},
        },
        buttonWidth,
        itemSpacingX);

    ToolbarSection::VerticalSeparator();

    const float dimensionOffsetComboWidth = ComputeComboPreviewWidth({"Várias", "Unidade", "Pixel"}, 104.0f);
    const float dimensionUnitComboWidth = ComputeComboPreviewWidth({"Várias", "mm", "cm", "m"}, 68.0f);
    const float dimensionControlWidth =
        std::max(
            ImGui::CalcTextSize("Offset").x,
            std::max(dimensionOffsetComboWidth, dimensionUnitComboWidth));
    const float dimensionButtonsWidth = buttonWidth * 2.0f + itemSpacingX;
    const float dimensionSectionWidth = ComputeDimensionSectionWidth(
        dimensionButtonsWidth,
        dimensionControlWidth);

    ToolbarSection::BeginSection("DimensionSection", dimensionSectionWidth);
    {
        auto queueDimensionOffsetModeEdit = [&](DimensionOffsetMode mode)
        {
            requests.dimensionToolStateSync.active = true;
            requests.dimensionToolStateSync.setOffsetMode = true;
            requests.dimensionToolStateSync.offsetMode = mode;

            if (editorState.selectionState.selection.dimensionIds.empty())
            {
                return;
            }

            requests.dimensionEdit.active = true;
            requests.dimensionEdit.setOffsetMode = true;
            requests.dimensionEdit.offsetMode = mode;
            requests.dimensionEdit.cameraZoom = static_cast<double>(cameraZoom);
            requests.dimensionEdit.dimensionIds.clear();
            requests.dimensionEdit.dimensionIds.reserve(editorState.selectionState.selection.dimensionIds.size());
            for (int dimensionId : editorState.selectionState.selection.dimensionIds)
            {
                if (projectDocument.FindDimensionById(dimensionId) != nullptr)
                {
                    requests.dimensionEdit.dimensionIds.push_back(dimensionId);
                }
            }
        };

        auto queueDimensionLengthUnitEdit = [&](LengthUnit unit)
        {
            requests.dimensionToolStateSync.active = true;
            requests.dimensionToolStateSync.setLengthUnit = true;
            requests.dimensionToolStateSync.lengthUnit = unit;

            if (editorState.selectionState.selection.dimensionIds.empty())
            {
                return;
            }

            requests.dimensionEdit.active = true;
            requests.dimensionEdit.setLengthUnit = true;
            requests.dimensionEdit.lengthUnit = unit;
            if (!requests.dimensionEdit.setOffsetMode)
            {
                requests.dimensionEdit.cameraZoom = static_cast<double>(cameraZoom);
            }
            requests.dimensionEdit.dimensionIds.clear();
            requests.dimensionEdit.dimensionIds.reserve(editorState.selectionState.selection.dimensionIds.size());
            for (int dimensionId : editorState.selectionState.selection.dimensionIds)
            {
                if (projectDocument.FindDimensionById(dimensionId) != nullptr)
                {
                    requests.dimensionEdit.dimensionIds.push_back(dimensionId);
                }
            }
        };

        auto getLengthUnitLabel = [](LengthUnit unit)
        {
            switch (unit)
            {
            case LengthUnit::Meter: return "m";
            case LengthUnit::Centimeter: return "cm";
            case LengthUnit::Millimeter: return "mm";
            default: return "m";
            }
        };

        auto getOffsetModeLabel = [](DimensionOffsetMode mode)
        {
            return mode == DimensionOffsetMode::ScreenPixels ? "Pixel" : "Unidade";
        };

        LengthUnit displayedLengthUnit = editorState.dimensionTool.newDimensionLengthUnit;
        DimensionOffsetMode displayedOffsetMode = editorState.dimensionTool.newDimensionOffsetMode;
        bool mixedLengthUnits = false;
        bool mixedOffsetModes = false;

        if (!editorState.selectionState.selection.dimensionIds.empty())
        {
            bool hasFirstDimension = false;
            for (int dimensionId : editorState.selectionState.selection.dimensionIds)
            {
                const Dimension* dimension = projectDocument.FindDimensionById(dimensionId);
                if (dimension == nullptr)
                {
                    continue;
                }

                if (!hasFirstDimension)
                {
                    displayedLengthUnit = dimension->lengthUnit;
                    displayedOffsetMode = dimension->offsetMode;
                    hasFirstDimension = true;
                    continue;
                }

                mixedLengthUnits = mixedLengthUnits || (displayedLengthUnit != dimension->lengthUnit);
                mixedOffsetModes = mixedOffsetModes || (displayedOffsetMode != dimension->offsetMode);
            }
        }

        const float buttonHeight =
            ToolbarSection::ClampFullHeightButtonHeight(ToolbarSection::GetButtonHeight("Cotas"));
        const float controlsTopY = ImGui::GetCursorPosY();
        ImGui::BeginGroup();
        ToolbarSection::DrawButton(
            {"Adicionar", true, [&]() { requests.editor = EditorRequest::ActivateAddDimensionTool; }, activeTool == EditorTool::AddDimension},
            ImVec2(buttonWidth, buttonHeight));

        ImGui::SameLine(0.0f, itemSpacingX);
        ToolbarSection::DrawButton(
            {"Ajustar", true, [&]() { requests.editor = EditorRequest::ActivateMoveDimensionTool; }, activeTool == EditorTool::MoveDimension},
            ImVec2(buttonWidth, buttonHeight));
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, itemSpacingX);
        ImGui::SetCursorPosY(std::max(0.0f, controlsTopY - dimensionControlVerticalOffset));
        ImGui::BeginGroup();
        const float adjustedDimensionComboTargetHeight =
            dimensionComboTargetHeight + ToolbarSection::GetTitleVerticalOffset("Cotas");
        const float comboFramePaddingY =
            std::max(1.0f, (adjustedDimensionComboTargetHeight - ImGui::GetFontSize()) * 0.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, dimensionControlItemSpacing);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, comboFramePaddingY));
        ImGui::TextUnformatted("Offset");
        const char* offsetModeLabel = mixedOffsetModes ? "Várias" : getOffsetModeLabel(displayedOffsetMode);
        ImGui::SetNextItemWidth(dimensionControlWidth);
        if (ImGui::BeginCombo("##DimensionOffsetMode", offsetModeLabel))
        {
            const bool isPixelMode = !mixedOffsetModes && displayedOffsetMode == DimensionOffsetMode::ScreenPixels;
            if (ImGui::Selectable("Pixel fixo", isPixelMode))
            {
                queueDimensionOffsetModeEdit(DimensionOffsetMode::ScreenPixels);
            }

            const bool isWorldMode = !mixedOffsetModes && displayedOffsetMode == DimensionOffsetMode::WorldUnits;
            if (ImGui::Selectable("Unidade fixa", isWorldMode))
            {
                queueDimensionOffsetModeEdit(DimensionOffsetMode::WorldUnits);
            }

            ImGui::EndCombo();
        }

        ImGui::TextUnformatted("Unid.");
        const char* lengthUnitLabel = mixedLengthUnits ? "Várias" : getLengthUnitLabel(displayedLengthUnit);
        ImGui::SetNextItemWidth(dimensionControlWidth);
        if (ImGui::BeginCombo("##DimensionLengthUnit", lengthUnitLabel))
        {
            if (ImGui::Selectable("m", !mixedLengthUnits && displayedLengthUnit == LengthUnit::Meter))
            {
                queueDimensionLengthUnitEdit(LengthUnit::Meter);
            }

            if (ImGui::Selectable("cm", !mixedLengthUnits && displayedLengthUnit == LengthUnit::Centimeter))
            {
                queueDimensionLengthUnitEdit(LengthUnit::Centimeter);
            }

            if (ImGui::Selectable("mm", !mixedLengthUnits && displayedLengthUnit == LengthUnit::Millimeter))
            {
                queueDimensionLengthUnitEdit(LengthUnit::Millimeter);
            }

            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);
        ImGui::EndGroup();

        ToolbarSection::PushTitleToBottom("Cotas");
    }
    ToolbarSection::EndSection("Cotas");

    ToolbarSection::VerticalSeparator();

    struct ToggleSpec
    {
        const char* label;
        bool* value;
    };

    ToggleSpec toggles[] = {
        {"Nós", &editorState.view.showNodes},
        {"Barras", &editorState.view.showBeams},
        {"Cotas", &editorState.view.showDimensions},
        {"Material", &editorState.view.showBeamMaterials},
        {"Seção", &editorState.view.showBeamSections},
        {"Sombras", &editorState.view.showShadows},
        {"Fade cota", &editorState.view.enableDimensionTextFade}
    };

    const int toggleCount = static_cast<int>(sizeof(toggles) / sizeof(toggles[0]));
    constexpr int rowCount = 3;
    const float expandedVisualizationItemSpacingY =
        rowCount > 1
            ? visualizationItemSpacing.y + toolbarContentHeightGainY / static_cast<float>(rowCount - 1)
            : visualizationItemSpacing.y;
    const int columnCount = (toggleCount + rowCount - 1) / rowCount;
    std::vector<float> visualizationColumnWidths(columnCount, 0.0f);
    for (int column = 0; column < columnCount; ++column)
    {
        for (int row = 0; row < rowCount; ++row)
        {
            const int index = column * rowCount + row;
            if (index >= toggleCount)
            {
                continue;
            }

            visualizationColumnWidths[column] = std::max(
                visualizationColumnWidths[column],
                ComputeCheckboxCellWidth(toggles[index].label, visualizationFramePadding));
        }
    }

    float visualizationSectionWidth = ToolbarSection::sectionPaddingX * 2.0f;
    for (float columnWidth : visualizationColumnWidths)
    {
        visualizationSectionWidth += columnWidth;
    }

    if (columnCount > 1)
    {
        visualizationSectionWidth += visualizationCellPadding.x * 2.0f * static_cast<float>(columnCount - 1);
    }

    ToolbarSection::BeginSection("ViewSection", visualizationSectionWidth);
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, visualizationFramePadding);
        ImGui::PushStyleVar(
            ImGuiStyleVar_ItemSpacing,
            ImVec2(visualizationItemSpacing.x, expandedVisualizationItemSpacingY));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, visualizationCellPadding);

        if (ImGui::BeginTable(
                "VisualizationTable",
                columnCount,
                ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX))
        {
            for (int column = 0; column < columnCount; ++column)
            {
                const std::string id = "##VisualizationCol" + std::to_string(column);
                ImGui::TableSetupColumn(
                    id.c_str(),
                    ImGuiTableColumnFlags_WidthFixed,
                    visualizationColumnWidths[column]);
            }

            for (int row = 0; row < rowCount; ++row)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < columnCount; ++column)
                {
                    const int index = column * rowCount + row;
                    ImGui::TableSetColumnIndex(column);
                    if (index >= toggleCount)
                    {
                        continue;
                    }

                    ImGui::Checkbox(toggles[index].label, toggles[index].value);
                }
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar(3);
        ToolbarSection::PushTitleToBottom("Visualização");
    }
    ToolbarSection::EndSection("Visualização");
    ToolbarSection::VerticalSeparator();
}

void TopToolbar::DrawLoadsTab(FrameRequests& requests, ProjectDocument& projectDocument, EditorState& editorState)
{
    const EditorTool activeTool = editorState.activeTool;
    NodalLoad pendingNodalLoad = editorState.loadTool.pendingNodalLoad;
    DistributedLoadValue pendingDistributedLoad = editorState.loadTool.pendingDistributedLoad;
    bool pendingDistributedLoadVariable = editorState.loadTool.pendingDistributedLoadVariable;
    std::uint64_t distributedLoadPanelSelectionToken = editorState.loadTool.distributedLoadPanelSelectionToken;
    bool distributedLoadPanelSelectionTokenInitialized =
        editorState.loadTool.distributedLoadPanelSelectionTokenInitialized;

    ToolbarSection::BeginSection("NodalLoadSection", 325.0f);
    {
        static LoadInputFieldState fxFieldState;
        static LoadInputFieldState fyFieldState;
        static LoadInputFieldState mzFieldState;

        const char* forceUnitLabel = UnitConversion::GetForceUnitLabel(projectDocument.displayUnits.force);
        const char* momentUnitLabel = UnitConversion::GetMomentUnitLabel(projectDocument.displayUnits.moment);

        auto buildSelectionState = [&](auto getValue, auto toDisplay) -> LoadComponentSelectionState
        {
            LoadComponentSelectionState selectionState;
            bool hasReferenceValue = false;
            double referenceValue = 0.0;

            for (int nodeId : editorState.selectionState.selection.nodeIds)
            {
                const Node* node = projectDocument.FindNodeById(nodeId);
                if (node == nullptr)
                {
                    continue;
                }

                const double displayValue = toDisplay(getValue(*node));
                if (!hasReferenceValue)
                {
                    referenceValue = displayValue;
                    hasReferenceValue = true;
                    continue;
                }

                if (std::abs(displayValue - referenceValue) > 1.0e-9)
                {
                    selectionState.hasSelection = true;
                    selectionState.isMixed = true;
                    selectionState.displayValue = 0.0;
                    return selectionState;
                }
            }

            if (hasReferenceValue)
            {
                selectionState.hasSelection = true;
                selectionState.isMixed = false;
                selectionState.displayValue = referenceValue;
            }

            return selectionState;
        };

        const LoadComponentSelectionState fxSelectionState = buildSelectionState(
            [](const Node& node) { return node.load.fx; },
            [&](double value) { return UnitConversion::ForceToDisplay(value, projectDocument.displayUnits.force); });

        const LoadComponentSelectionState fySelectionState = buildSelectionState(
            [](const Node& node) { return node.load.fy; },
            [&](double value) { return UnitConversion::ForceToDisplay(value, projectDocument.displayUnits.force); });

        const LoadComponentSelectionState mzSelectionState = buildSelectionState(
            [](const Node& node) { return node.load.mz; },
            [&](double value) { return UnitConversion::MomentToDisplay(value, projectDocument.displayUnits.moment); });

        if (fxSelectionState.hasSelection && !fxSelectionState.isMixed)
        {
            pendingNodalLoad.fx = UnitConversion::ForceToSI(
                fxSelectionState.displayValue,
                projectDocument.displayUnits.force);
        }

        if (fySelectionState.hasSelection && !fySelectionState.isMixed)
        {
            pendingNodalLoad.fy = UnitConversion::ForceToSI(
                fySelectionState.displayValue,
                projectDocument.displayUnits.force);
        }

        if (mzSelectionState.hasSelection && !mzSelectionState.isMixed)
        {
            pendingNodalLoad.mz = UnitConversion::MomentToSI(
                mzSelectionState.displayValue,
                projectDocument.displayUnits.moment);
        }

        const double displayFx = fxSelectionState.hasSelection
            ? fxSelectionState.displayValue
            : UnitConversion::ForceToDisplay(
                  pendingNodalLoad.fx,
                  projectDocument.displayUnits.force);

        const double displayFy = fySelectionState.hasSelection
            ? fySelectionState.displayValue
            : UnitConversion::ForceToDisplay(
                  pendingNodalLoad.fy,
                  projectDocument.displayUnits.force);

        const double displayMz = mzSelectionState.hasSelection
            ? mzSelectionState.displayValue
            : UnitConversion::MomentToDisplay(
                  pendingNodalLoad.mz,
                  projectDocument.displayUnits.moment);

        SetLoadInputText(
            fxFieldState,
            fxSelectionState.hasSelection && fxSelectionState.isMixed,
            displayFx);
        SetLoadInputText(
            fyFieldState,
            fySelectionState.hasSelection && fySelectionState.isMixed,
            displayFy);
        SetLoadInputText(
            mzFieldState,
            mzSelectionState.hasSelection && mzSelectionState.isMixed,
            displayMz);

        const float labelWidth = 22.0f;
        const float inputWidth = 50.0f;
        const float unitOffset = 8.0f;
        const float gapToButtons = 18.0f;
        const float buttonWidthLocal = 90.0f;

        const float spacingY = ImGui::GetStyle().ItemSpacing.y;
        const float totalRowsHeight = ToolbarSection::GetButtonHeight("Concentrado em nó");
        const float tallButtonHeight = ToolbarSection::ClampFullHeightButtonHeight(totalRowsHeight);

        const float rowHeight = (totalRowsHeight - 2.0f * spacingY) / 3.0f;

        float framePaddingY = (rowHeight - ImGui::GetFontSize()) * 0.5f;
        if (framePaddingY < 0.0f)
        {
            framePaddingY = 0.0f;
        }

        const float startX = ImGui::GetCursorPosX();
        const float buttonsX = startX + labelWidth + inputWidth + 40.0f + gapToButtons;

        auto enqueueNodalLoadEdit = [&](const auto& configureRequest)
        {
            if (!editorState.selectionState.selection.HasNodes())
            {
                return;
            }

            FrameRequests::NodalLoadSelectionEditRequest editRequest;
            editRequest.active = true;
            editRequest.nodeIds.reserve(editorState.selectionState.selection.nodeIds.size());
            for (int nodeId : editorState.selectionState.selection.nodeIds)
            {
                if (projectDocument.FindNodeById(nodeId) != nullptr)
                {
                    editRequest.nodeIds.push_back(nodeId);
                }
            }

            if (editRequest.nodeIds.empty())
            {
                return;
            }

            configureRequest(editRequest);
            requests.nodalLoadEdit = std::move(editRequest);
        };

        ImGui::PushStyleVar(
            ImGuiStyleVar_FramePadding,
            ImVec2(ImGui::GetStyle().FramePadding.x, framePaddingY));

        auto drawLoadRow = [&](const char* label,
                               const char* inputId,
                               LoadInputFieldState& fieldState,
                               const char* unitLabel,
                               auto toSI,
                               auto applyToRequest,
                               double& pendingComponent)
        {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(label);

            ImGui::SameLine(startX + labelWidth);
            ImGui::SetNextItemWidth(inputWidth);
            ImGui::InputText(
                inputId,
                fieldState.text,
                sizeof(fieldState.text),
                ImGuiInputTextFlags_CharsScientific | ImGuiInputTextFlags_AutoSelectAll);

            double parsedDisplayValue = 0.0;
            if (ImGui::IsItemEdited() && TryParseTextToDouble(fieldState.text, parsedDisplayValue))
            {
                const double componentValue = toSI(parsedDisplayValue);
                pendingComponent = componentValue;
                enqueueNodalLoadEdit([&](FrameRequests::NodalLoadSelectionEditRequest& editRequest)
                {
                    applyToRequest(editRequest, componentValue);
                });
            }

            fieldState.isEditing = ImGui::IsItemActive();

            ImGui::SameLine(startX + labelWidth + inputWidth + unitOffset);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(unitLabel);
        };

        const float fieldsTopY = ImGui::GetCursorPosY();

        drawLoadRow(
            "Fx",
            "##Fx",
            fxFieldState,
            forceUnitLabel,
            [&](double displayValue) { return UnitConversion::ForceToSI(displayValue, projectDocument.displayUnits.force); },
            [](FrameRequests::NodalLoadSelectionEditRequest& request, double value)
            {
                request.setFx = true;
                request.fx = value;
            },
            pendingNodalLoad.fx);
        drawLoadRow(
            "Fy",
            "##Fy",
            fyFieldState,
            forceUnitLabel,
            [&](double displayValue) { return UnitConversion::ForceToSI(displayValue, projectDocument.displayUnits.force); },
            [](FrameRequests::NodalLoadSelectionEditRequest& request, double value)
            {
                request.setFy = true;
                request.fy = value;
            },
            pendingNodalLoad.fy);
        drawLoadRow(
            "Mz",
            "##Mz",
            mzFieldState,
            momentUnitLabel,
            [&](double displayValue) { return UnitConversion::MomentToSI(displayValue, projectDocument.displayUnits.moment); },
            [](FrameRequests::NodalLoadSelectionEditRequest& request, double value)
            {
                request.setMz = true;
                request.mz = value;
            },
            pendingNodalLoad.mz);

        ImGui::PopStyleVar();

        ImGui::SetCursorPos(ImVec2(buttonsX, fieldsTopY));

        ToolbarSection::DrawButton(
            {"Adicionar", true, [&]() { requests.editor = EditorRequest::ActivateAddPointLoadTool; }, activeTool == EditorTool::AddPointLoad},
            ImVec2(buttonWidthLocal, tallButtonHeight));

        ImGui::SameLine();

        ToolbarSection::DrawButton(
            {"Remover", true, [&]() { requests.editor = EditorRequest::ActivateRemovePointLoadTool; }, activeTool == EditorTool::RemovePointLoad},
            ImVec2(buttonWidthLocal, tallButtonHeight));
        ToolbarSection::PushTitleToBottom("Concentrado em nó");
    }
    ToolbarSection::EndSection("Concentrado em nó");
    ToolbarSection::VerticalSeparator();

    ToolbarSection::BeginSection(
        "DistributedLoadSection",
        std::max(1.0f, 430.0f - loadTabSectionTrailingTrimX));
    {
        static LoadInputFieldState qxStartFieldState;
        static LoadInputFieldState qyStartFieldState;
        static LoadInputFieldState qxEndFieldState;
        static LoadInputFieldState qyEndFieldState;

        const char* distributedUnitLabel =
            UnitConversion::GetDistributedLoadUnitLabel(projectDocument.displayUnits.distributedLoad);

        auto getBeamLoadValue = [&](int beamId) -> DistributedLoadValue
        {
            const BeamDistributedLoad* distributedLoad = projectDocument.FindDistributedLoadByBeamId(beamId);
            return distributedLoad != nullptr ? distributedLoad->value : DistributedLoadValue{};
        };

        auto buildDistributedSelectionState = [&](auto getValue) -> LoadComponentSelectionState
        {
            LoadComponentSelectionState selectionState;
            bool hasReferenceValue = false;
            double referenceValue = 0.0;

            for (int beamId : editorState.selectionState.selection.beamIds)
            {
                if (projectDocument.FindBeamById(beamId) == nullptr)
                {
                    continue;
                }

                const double displayValue = UnitConversion::DistributedLoadToDisplay(
                    getValue(getBeamLoadValue(beamId)),
                    projectDocument.displayUnits.distributedLoad);
                if (!hasReferenceValue)
                {
                    referenceValue = displayValue;
                    hasReferenceValue = true;
                    continue;
                }

                if (std::abs(displayValue - referenceValue) > 1.0e-9)
                {
                    selectionState.hasSelection = true;
                    selectionState.isMixed = true;
                    selectionState.displayValue = 0.0;
                    return selectionState;
                }
            }

            if (hasReferenceValue)
            {
                selectionState.hasSelection = true;
                selectionState.isMixed = false;
                selectionState.displayValue = referenceValue;
            }

            return selectionState;
        };

        bool hasDistributedSelection = false;
        bool mixedDistributedVariable = false;
        bool referenceDistributedVariable = false;
        for (int beamId : editorState.selectionState.selection.beamIds)
        {
            if (projectDocument.FindBeamById(beamId) == nullptr)
            {
                continue;
            }

            const bool isVariable = getBeamLoadValue(beamId).IsVariable();
            if (!hasDistributedSelection)
            {
                hasDistributedSelection = true;
                referenceDistributedVariable = isVariable;
                continue;
            }

            if (referenceDistributedVariable != isVariable)
            {
                mixedDistributedVariable = true;
                break;
            }
        }

        const bool shouldSyncDistributedVariableFromSelection =
            hasDistributedSelection &&
            (!distributedLoadPanelSelectionTokenInitialized ||
             distributedLoadPanelSelectionToken !=
                 ComputeBeamSelectionToken(projectDocument, editorState.selectionState.selection));

        if (!hasDistributedSelection)
        {
            distributedLoadPanelSelectionTokenInitialized = false;
        }
        else
        {
            distributedLoadPanelSelectionToken =
                ComputeBeamSelectionToken(projectDocument, editorState.selectionState.selection);
            distributedLoadPanelSelectionTokenInitialized = true;
        }

        if (shouldSyncDistributedVariableFromSelection && !mixedDistributedVariable)
        {
            pendingDistributedLoadVariable = referenceDistributedVariable;
        }

        const LoadComponentSelectionState qxStartSelectionState = buildDistributedSelectionState(
            [](const DistributedLoadValue& load) { return load.qxStart; });
        const LoadComponentSelectionState qyStartSelectionState = buildDistributedSelectionState(
            [](const DistributedLoadValue& load) { return load.qyStart; });
        const LoadComponentSelectionState qxEndSelectionState = buildDistributedSelectionState(
            [](const DistributedLoadValue& load) { return load.qxEnd; });
        const LoadComponentSelectionState qyEndSelectionState = buildDistributedSelectionState(
            [](const DistributedLoadValue& load) { return load.qyEnd; });

        if (qxStartSelectionState.hasSelection && !qxStartSelectionState.isMixed)
        {
            pendingDistributedLoad.qxStart = UnitConversion::DistributedLoadToSI(
                qxStartSelectionState.displayValue,
                projectDocument.displayUnits.distributedLoad);
        }
        if (qyStartSelectionState.hasSelection && !qyStartSelectionState.isMixed)
        {
            pendingDistributedLoad.qyStart = UnitConversion::DistributedLoadToSI(
                qyStartSelectionState.displayValue,
                projectDocument.displayUnits.distributedLoad);
        }
        if (qxEndSelectionState.hasSelection && !qxEndSelectionState.isMixed)
        {
            pendingDistributedLoad.qxEnd = UnitConversion::DistributedLoadToSI(
                qxEndSelectionState.displayValue,
                projectDocument.displayUnits.distributedLoad);
        }
        if (qyEndSelectionState.hasSelection && !qyEndSelectionState.isMixed)
        {
            pendingDistributedLoad.qyEnd = UnitConversion::DistributedLoadToSI(
                qyEndSelectionState.displayValue,
                projectDocument.displayUnits.distributedLoad);
        }

        if (!pendingDistributedLoadVariable)
        {
            pendingDistributedLoad.qxEnd = pendingDistributedLoad.qxStart;
            pendingDistributedLoad.qyEnd = pendingDistributedLoad.qyStart;
        }

        const double displayQxStart = qxStartSelectionState.hasSelection
            ? qxStartSelectionState.displayValue
            : UnitConversion::DistributedLoadToDisplay(
                  pendingDistributedLoad.qxStart,
                  projectDocument.displayUnits.distributedLoad);
        const double displayQyStart = qyStartSelectionState.hasSelection
            ? qyStartSelectionState.displayValue
            : UnitConversion::DistributedLoadToDisplay(
                  pendingDistributedLoad.qyStart,
                  projectDocument.displayUnits.distributedLoad);
        const double displayQxEnd = qxEndSelectionState.hasSelection
            ? qxEndSelectionState.displayValue
            : UnitConversion::DistributedLoadToDisplay(
                  pendingDistributedLoad.qxEnd,
                  projectDocument.displayUnits.distributedLoad);
        const double displayQyEnd = qyEndSelectionState.hasSelection
            ? qyEndSelectionState.displayValue
            : UnitConversion::DistributedLoadToDisplay(
                  pendingDistributedLoad.qyEnd,
                  projectDocument.displayUnits.distributedLoad);

        SetLoadInputText(
            qxStartFieldState,
            qxStartSelectionState.hasSelection && qxStartSelectionState.isMixed,
            displayQxStart);
        SetLoadInputText(
            qyStartFieldState,
            qyStartSelectionState.hasSelection && qyStartSelectionState.isMixed,
            displayQyStart);
        SetLoadInputText(
            qxEndFieldState,
            qxEndSelectionState.hasSelection && qxEndSelectionState.isMixed,
            displayQxEnd);
        SetLoadInputText(
            qyEndFieldState,
            qyEndSelectionState.hasSelection && qyEndSelectionState.isMixed,
            displayQyEnd);

        auto enqueueDistributedLoadEdit = [&](const auto& configureRequest)
        {
            if (!editorState.selectionState.selection.HasBeams())
            {
                return;
            }

            FrameRequests::DistributedLoadSelectionEditRequest editRequest;
            editRequest.active = true;
            editRequest.beamIds.reserve(editorState.selectionState.selection.beamIds.size());
            for (int beamId : editorState.selectionState.selection.beamIds)
            {
                if (projectDocument.FindBeamById(beamId) == nullptr)
                {
                    continue;
                }

                editRequest.beamIds.push_back(beamId);
            }

            if (editRequest.beamIds.empty())
            {
                return;
            }

            configureRequest(editRequest);
            requests.distributedLoadEdit = std::move(editRequest);
        };

        const float buttonWidthLocal = 90.0f;
        const float fieldSpacingX = 6.0f;
        const float labelWidth = std::max(
            ImGui::CalcTextSize("Qx:").x,
            ImGui::CalcTextSize("Qy:").x) + fieldSpacingX;
        const float startEndpointLabelWidth = ImGui::CalcTextSize("Ini").x;
        const float endEndpointLabelWidth = ImGui::CalcTextSize("Fim").x;
        const float inputWidth = 52.0f;
        const float unitWidth = std::max(
            ImGui::CalcTextSize("N/m").x,
            ImGui::CalcTextSize("kN/m").x);
        const float gapToButtons = 8.0f;
        const float leftPaneWidth =
            labelWidth +
            startEndpointLabelWidth +
            fieldSpacingX +
            inputWidth +
            fieldSpacingX +
            endEndpointLabelWidth +
            fieldSpacingX +
            inputWidth +
            fieldSpacingX +
            unitWidth;
        const float totalRowsHeight = ToolbarSection::GetButtonHeight("Distribuída na barra");
        const float tallButtonHeight = ToolbarSection::ClampFullHeightButtonHeight(totalRowsHeight);
        const float buttonsX = ImGui::GetCursorPosX() + leftPaneWidth + gapToButtons;
        const float contentTopY = ImGui::GetCursorPosY();

        const bool variableChanged = ImGui::Checkbox("Variável", &pendingDistributedLoadVariable);
        if (variableChanged && !pendingDistributedLoadVariable)
        {
            pendingDistributedLoad.qxEnd = pendingDistributedLoad.qxStart;
            pendingDistributedLoad.qyEnd = pendingDistributedLoad.qyStart;
            enqueueDistributedLoadEdit([&](FrameRequests::DistributedLoadSelectionEditRequest& editRequest)
            {
                editRequest.setQxEnd = true;
                editRequest.setQyEnd = true;
                editRequest.qxEnd = pendingDistributedLoad.qxStart;
                editRequest.qyEnd = pendingDistributedLoad.qyStart;
            });
        }

        const float tableTopY = ImGui::GetCursorPosY() + 4.0f;
        ImGui::SetCursorPosY(tableTopY);
        if (ImGui::BeginTable(
                "DistributedLoadInputTable",
                6,
                ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX))
        {
            ImGui::TableSetupColumn("##DistributedLoadLabel", ImGuiTableColumnFlags_WidthFixed, labelWidth);
            ImGui::TableSetupColumn("##DistributedLoadStartLabel", ImGuiTableColumnFlags_WidthFixed, startEndpointLabelWidth + fieldSpacingX);
            ImGui::TableSetupColumn("##DistributedLoadStart", ImGuiTableColumnFlags_WidthFixed, inputWidth + fieldSpacingX);
            ImGui::TableSetupColumn("##DistributedLoadEndLabel", ImGuiTableColumnFlags_WidthFixed, endEndpointLabelWidth + fieldSpacingX);
            ImGui::TableSetupColumn("##DistributedLoadEnd", ImGuiTableColumnFlags_WidthFixed, inputWidth + fieldSpacingX);
            ImGui::TableSetupColumn("##DistributedLoadUnit", ImGuiTableColumnFlags_WidthFixed, unitWidth);

            auto drawDistributedFieldRow = [&](const char* label,
                                               const char* startId,
                                               LoadInputFieldState& startFieldState,
                                               double& pendingStartValue,
                                               const auto& applyStartValue,
                                               const char* endId,
                                               LoadInputFieldState& endFieldState,
                                               double& pendingEndValue,
                                               const auto& applyEndValue)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(label);

                ImGui::TableSetColumnIndex(1);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Ini");

                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(inputWidth);
                ImGui::InputText(
                    startId,
                    startFieldState.text,
                    sizeof(startFieldState.text),
                    ImGuiInputTextFlags_CharsScientific | ImGuiInputTextFlags_AutoSelectAll);

                double parsedStartValue = 0.0;
                if (ImGui::IsItemEdited() && TryParseTextToDouble(startFieldState.text, parsedStartValue))
                {
                    const double startValue = UnitConversion::DistributedLoadToSI(
                        parsedStartValue,
                        projectDocument.displayUnits.distributedLoad);
                    pendingStartValue = startValue;
                    if (!pendingDistributedLoadVariable)
                    {
                        pendingEndValue = startValue;
                    }

                    enqueueDistributedLoadEdit([&](FrameRequests::DistributedLoadSelectionEditRequest& editRequest)
                    {
                        applyStartValue(editRequest, startValue);
                        if (!pendingDistributedLoadVariable)
                        {
                            applyEndValue(editRequest, startValue);
                        }
                    });
                }
                startFieldState.isEditing = ImGui::IsItemActive();

                ImGui::TableSetColumnIndex(3);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Fim");

                ImGui::TableSetColumnIndex(4);
                if (!pendingDistributedLoadVariable)
                {
                    ImGui::BeginDisabled();
                }

                ImGui::SetNextItemWidth(inputWidth);
                ImGui::InputText(
                    endId,
                    endFieldState.text,
                    sizeof(endFieldState.text),
                    ImGuiInputTextFlags_CharsScientific | ImGuiInputTextFlags_AutoSelectAll);

                double parsedEndValue = 0.0;
                if (pendingDistributedLoadVariable &&
                    ImGui::IsItemEdited() &&
                    TryParseTextToDouble(endFieldState.text, parsedEndValue))
                {
                    const double endValue = UnitConversion::DistributedLoadToSI(
                        parsedEndValue,
                        projectDocument.displayUnits.distributedLoad);
                    pendingEndValue = endValue;

                    enqueueDistributedLoadEdit([&](FrameRequests::DistributedLoadSelectionEditRequest& editRequest)
                    {
                        applyEndValue(editRequest, endValue);
                    });
                }
                endFieldState.isEditing =
                    pendingDistributedLoadVariable && ImGui::IsItemActive();

                if (!pendingDistributedLoadVariable)
                {
                    ImGui::EndDisabled();
                }

                ImGui::TableSetColumnIndex(5);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(distributedUnitLabel);
            };

            drawDistributedFieldRow(
                "Qx:",
                "##DistributedQxStart",
                qxStartFieldState,
                pendingDistributedLoad.qxStart,
                [](FrameRequests::DistributedLoadSelectionEditRequest& request, double value)
                {
                    request.setQxStart = true;
                    request.qxStart = value;
                },
                "##DistributedQxEnd",
                qxEndFieldState,
                pendingDistributedLoad.qxEnd,
                [](FrameRequests::DistributedLoadSelectionEditRequest& request, double value)
                {
                    request.setQxEnd = true;
                    request.qxEnd = value;
                });
            drawDistributedFieldRow(
                "Qy:",
                "##DistributedQyStart",
                qyStartFieldState,
                pendingDistributedLoad.qyStart,
                [](FrameRequests::DistributedLoadSelectionEditRequest& request, double value)
                {
                    request.setQyStart = true;
                    request.qyStart = value;
                },
                "##DistributedQyEnd",
                qyEndFieldState,
                pendingDistributedLoad.qyEnd,
                [](FrameRequests::DistributedLoadSelectionEditRequest& request, double value)
                {
                    request.setQyEnd = true;
                    request.qyEnd = value;
                });

            ImGui::EndTable();
        }

        ImGui::SetCursorPos(ImVec2(buttonsX, contentTopY));
        ToolbarSection::DrawButton(
            {"Adicionar", true, [&]() { requests.editor = EditorRequest::ActivateAddDistributedLoadTool; }, activeTool == EditorTool::AddDistributedLoad},
            ImVec2(buttonWidthLocal, tallButtonHeight));

        ImGui::SameLine();

        ToolbarSection::DrawButton(
            {"Remover", true, [&]() { requests.editor = EditorRequest::ActivateRemoveDistributedLoadTool; }, activeTool == EditorTool::RemoveDistributedLoad},
            ImVec2(buttonWidthLocal, tallButtonHeight));

        ToolbarSection::PushTitleToBottom("Distribuída na barra");
    }
    ToolbarSection::EndSection("Distribuída na barra");
    ToolbarSection::VerticalSeparator();

    const char* loadViewPixelsUnitLabel = "px";
    constexpr float loadViewSliderWidth = 24.0f;
    constexpr float loadViewSliderSpacing = 10.0f;
    constexpr float loadViewGroupSpacing = 12.0f;
    constexpr float loadViewLimitInputWidth = 48.0f;
    constexpr float loadViewSliderLimitMaximum = 150.0f;
    constexpr float loadViewInlineGap = 3.0f;
    const ImVec2 loadViewTableCellPadding = ImVec2(
        visualizationCellPadding.x + 3.0f,
        visualizationCellPadding.y);

    struct LoadToggleSpec
    {
        const char* label;
        bool* value;
    };

    LoadToggleSpec loadToggles[] = {
        {"Visualizar", &editorState.view.showPointLoads},
        {"Ocultar original", &editorState.view.hideOriginalStructure},
        {"F Resultante", &editorState.view.showPointLoadResultant},
        {"Q Resultante", &editorState.view.showDistributedLoadResultant},
        {"Suprimir texto", &editorState.view.suppressPointLoadText},
        {"Preenchimento", &editorState.view.showDistributedLoadArea}
    };

    const int loadToggleCount = static_cast<int>(sizeof(loadToggles) / sizeof(loadToggles[0]));
    constexpr int loadToggleRowCount = 3;
    const float expandedLoadVisualizationItemSpacingY =
        loadToggleRowCount > 1
            ? visualizationItemSpacing.y + toolbarContentHeightGainY / static_cast<float>(loadToggleRowCount - 1)
            : visualizationItemSpacing.y;
    const int loadToggleColumnCount = (loadToggleCount + loadToggleRowCount - 1) / loadToggleRowCount;
    std::vector<float> loadToggleColumnWidths(loadToggleColumnCount, 0.0f);
    for (int column = 0; column < loadToggleColumnCount; ++column)
    {
        for (int row = 0; row < loadToggleRowCount; ++row)
        {
            const int index = column * loadToggleRowCount + row;
            if (index >= loadToggleCount)
            {
                continue;
            }

            loadToggleColumnWidths[column] = std::max(
                loadToggleColumnWidths[column],
                ComputeCheckboxCellWidth(loadToggles[index].label, visualizationFramePadding));
        }
    }

    float loadToggleSectionWidth = 0.0f;
    for (float columnWidth : loadToggleColumnWidths)
    {
        loadToggleSectionWidth += columnWidth;
    }
    if (loadToggleColumnCount > 1)
    {
        loadToggleSectionWidth += visualizationCellPadding.x * 2.0f * static_cast<float>(loadToggleColumnCount - 1);
    }
    const float loadToggleRowHeight = ImGui::GetFontSize() + visualizationFramePadding.y * 2.0f;
    const float loadToggleContentHeight =
        loadToggleRowHeight * static_cast<float>(loadToggleRowCount) +
        expandedLoadVisualizationItemSpacingY * static_cast<float>(loadToggleRowCount - 1);
    const float loadSliderLabelHeight = ImGui::GetTextLineHeight();
    float loadViewSliderHeight =
        loadToggleContentHeight - loadSliderLabelHeight - ImGui::GetStyle().ItemSpacing.y;
    if (loadViewSliderHeight < 1.0f)
    {
        loadViewSliderHeight = 1.0f;
    }

    const float sliderStackWidth =
        loadViewSliderWidth * 3.0f + loadViewSliderSpacing * 2.0f;
    const float scaleFieldLabelTextWidth =
        std::max({
            ImGui::CalcTextSize("Fmáx").x,
            ImGui::CalcTextSize("Mmáx").x,
            ImGui::CalcTextSize("Qmáx").x});
    const float scaleFieldLabelWidth = scaleFieldLabelTextWidth + loadViewInlineGap;
    const float scaleFieldInputColumnWidth = loadViewLimitInputWidth + loadViewInlineGap;
    const float scaleFieldUnitWidth = ImGui::CalcTextSize(loadViewPixelsUnitLabel).x;
    const float scaleFieldGroupWidth =
        scaleFieldLabelWidth +
        scaleFieldInputColumnWidth +
        scaleFieldUnitWidth;

    const float forceUnitComboWidth = ComputeComboPreviewWidth({"N", "kN"}, 56.0f);
    const float momentUnitComboWidth = ComputeComboPreviewWidth({"N.m", "kN.m"}, 68.0f);
    const float distributedLoadComboWidth = ComputeComboPreviewWidth({"N/m", "kN/m"}, 68.0f);
    const float loadUnitComboWidth = std::max({
        forceUnitComboWidth,
        momentUnitComboWidth,
        distributedLoadComboWidth});
    const float loadUnitLabelTextWidth = std::max({
        ImGui::CalcTextSize("Força").x,
        ImGui::CalcTextSize("Momento").x,
        ImGui::CalcTextSize("Distribuída").x});
    const float loadUnitLabelWidth = loadUnitLabelTextWidth + loadViewInlineGap;
    const float loadUnitGroupWidth =
        loadUnitLabelWidth +
        loadUnitComboWidth;

    const float loadViewSectionWidth =
        ToolbarSection::sectionPaddingX * 2.0f +
        loadToggleSectionWidth +
        loadViewGroupSpacing +
        sliderStackWidth +
        loadViewGroupSpacing +
        scaleFieldGroupWidth +
        loadViewGroupSpacing +
        loadUnitGroupWidth +
        0.0f -
        loadTabSectionTrailingTrimX;

    ToolbarSection::BeginSection("LoadViewSection", loadViewSectionWidth);
    {
        editorState.view.pointLoadForceSliderLimitPixels = std::clamp(
            editorState.view.pointLoadForceSliderLimitPixels,
            1.0f,
            loadViewSliderLimitMaximum);
        editorState.view.pointLoadMomentSliderLimitPixels = std::clamp(
            editorState.view.pointLoadMomentSliderLimitPixels,
            1.0f,
            loadViewSliderLimitMaximum);
        editorState.view.distributedLoadSliderLimitPixels = std::clamp(
            editorState.view.distributedLoadSliderLimitPixels,
            1.0f,
            loadViewSliderLimitMaximum);
        editorState.view.pointLoadForceMaxPixels = std::clamp(
            editorState.view.pointLoadForceMaxPixels,
            1.0f,
            editorState.view.pointLoadForceSliderLimitPixels);
        editorState.view.pointLoadMomentMaxPixels = std::clamp(
            editorState.view.pointLoadMomentMaxPixels,
            1.0f,
            editorState.view.pointLoadMomentSliderLimitPixels);
        editorState.view.distributedLoadMaxPixels = std::clamp(
            editorState.view.distributedLoadMaxPixels,
            1.0f,
            editorState.view.distributedLoadSliderLimitPixels);

        ImGui::BeginGroup();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, visualizationFramePadding);
        ImGui::PushStyleVar(
            ImGuiStyleVar_ItemSpacing,
            ImVec2(visualizationItemSpacing.x, expandedLoadVisualizationItemSpacingY));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, visualizationCellPadding);

        if (ImGui::BeginTable(
                "LoadVisualizationToggleTable",
                loadToggleColumnCount,
                ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX))
        {
            for (int column = 0; column < loadToggleColumnCount; ++column)
            {
                const std::string id = "##LoadVisualizationToggleCol" + std::to_string(column);
                ImGui::TableSetupColumn(
                    id.c_str(),
                    ImGuiTableColumnFlags_WidthFixed,
                    loadToggleColumnWidths[column]);
            }

            for (int row = 0; row < loadToggleRowCount; ++row)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < loadToggleColumnCount; ++column)
                {
                    const int index = column * loadToggleRowCount + row;
                    ImGui::TableSetColumnIndex(column);
                    if (index >= loadToggleCount)
                    {
                        continue;
                    }

                    ImGui::Checkbox(loadToggles[index].label, loadToggles[index].value);
                }
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar(3);
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, loadViewGroupSpacing);

        ImGui::BeginGroup();
        ImGui::BeginGroup();
        {
            const float forceSliderStartX = ImGui::GetCursorPosX();
            ImGui::VSliderFloat(
                "##PointLoadForceMaxSlider",
                ImVec2(loadViewSliderWidth, loadViewSliderHeight),
                &editorState.view.pointLoadForceMaxPixels,
                1.0f,
                editorState.view.pointLoadForceSliderLimitPixels,
                "");

            const float forceLabelWidth = ImGui::CalcTextSize("F").x;
            ImGui::SetCursorPosX(
                forceSliderStartX +
                std::max(0.0f, (loadViewSliderWidth - forceLabelWidth) * 0.5f));
            ImGui::TextUnformatted("F");
        }
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, loadViewSliderSpacing);

        ImGui::BeginGroup();
        {
            const float momentSliderStartX = ImGui::GetCursorPosX();
            ImGui::VSliderFloat(
                "##PointLoadMomentMaxSlider",
                ImVec2(loadViewSliderWidth, loadViewSliderHeight),
                &editorState.view.pointLoadMomentMaxPixels,
                1.0f,
                editorState.view.pointLoadMomentSliderLimitPixels,
                "");

            const float momentLabelWidth = ImGui::CalcTextSize("M").x;
            ImGui::SetCursorPosX(
                momentSliderStartX +
                std::max(0.0f, (loadViewSliderWidth - momentLabelWidth) * 0.5f));
            ImGui::TextUnformatted("M");
        }
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, loadViewSliderSpacing);

        ImGui::BeginGroup();
        {
            const float distributedSliderStartX = ImGui::GetCursorPosX();
            ImGui::VSliderFloat(
                "##DistributedLoadMaxSlider",
                ImVec2(loadViewSliderWidth, loadViewSliderHeight),
                &editorState.view.distributedLoadMaxPixels,
                1.0f,
                editorState.view.distributedLoadSliderLimitPixels,
                "");

            const float distributedLabelWidth = ImGui::CalcTextSize("Q").x;
            ImGui::SetCursorPosX(
                distributedSliderStartX +
                std::max(0.0f, (loadViewSliderWidth - distributedLabelWidth) * 0.5f));
            ImGui::TextUnformatted("Q");
        }
        ImGui::EndGroup();
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, loadViewGroupSpacing);

        ImGui::BeginGroup();
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, loadViewTableCellPadding);
        if (ImGui::BeginTable(
                "LoadSliderLimitTable",
                3,
                ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX))
        {
            ImGui::TableSetupColumn(
                "##LoadSliderLimitLabel",
                ImGuiTableColumnFlags_WidthFixed,
                scaleFieldLabelWidth);
            ImGui::TableSetupColumn(
                "##LoadSliderLimitInput",
                ImGuiTableColumnFlags_WidthFixed,
                scaleFieldInputColumnWidth);
            ImGui::TableSetupColumn(
                "##LoadSliderLimitUnit",
                ImGuiTableColumnFlags_WidthFixed,
                scaleFieldUnitWidth);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Fmáx");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(loadViewLimitInputWidth);
            if (ImGui::InputFloat(
                    "##PointLoadForceSliderLimitPixels",
                    &editorState.view.pointLoadForceSliderLimitPixels,
                    0.0f,
                    0.0f,
                    "%.0f"))
            {
                editorState.view.pointLoadForceSliderLimitPixels = std::clamp(
                    editorState.view.pointLoadForceSliderLimitPixels,
                    1.0f,
                    loadViewSliderLimitMaximum);
                editorState.view.pointLoadForceMaxPixels = std::min(
                    editorState.view.pointLoadForceMaxPixels,
                    editorState.view.pointLoadForceSliderLimitPixels);
            }
            ImGui::TableSetColumnIndex(2);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(loadViewPixelsUnitLabel);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Mmáx");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(loadViewLimitInputWidth);
            if (ImGui::InputFloat(
                    "##PointLoadMomentSliderLimitPixels",
                    &editorState.view.pointLoadMomentSliderLimitPixels,
                    0.0f,
                    0.0f,
                    "%.0f"))
            {
                editorState.view.pointLoadMomentSliderLimitPixels = std::clamp(
                    editorState.view.pointLoadMomentSliderLimitPixels,
                    1.0f,
                    loadViewSliderLimitMaximum);
                editorState.view.pointLoadMomentMaxPixels = std::min(
                    editorState.view.pointLoadMomentMaxPixels,
                    editorState.view.pointLoadMomentSliderLimitPixels);
            }
            ImGui::TableSetColumnIndex(2);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(loadViewPixelsUnitLabel);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Qmáx");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(loadViewLimitInputWidth);
            if (ImGui::InputFloat(
                    "##DistributedLoadSliderLimitPixels",
                    &editorState.view.distributedLoadSliderLimitPixels,
                    0.0f,
                    0.0f,
                    "%.0f"))
            {
                editorState.view.distributedLoadSliderLimitPixels = std::clamp(
                    editorState.view.distributedLoadSliderLimitPixels,
                    1.0f,
                    loadViewSliderLimitMaximum);
                editorState.view.distributedLoadMaxPixels = std::min(
                    editorState.view.distributedLoadMaxPixels,
                    editorState.view.distributedLoadSliderLimitPixels);
            }
            ImGui::TableSetColumnIndex(2);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(loadViewPixelsUnitLabel);

            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, loadViewGroupSpacing);

        ImGui::BeginGroup();
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, loadViewTableCellPadding);
        DisplayUnits updatedDisplayUnits = projectDocument.displayUnits;
        bool displayUnitsChanged = false;
        if (ImGui::BeginTable(
                "LoadDisplayUnitsTable",
                2,
                ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX))
        {
            ImGui::TableSetupColumn(
                "##LoadDisplayUnitLabel",
                ImGuiTableColumnFlags_WidthFixed,
                loadUnitLabelWidth);
            ImGui::TableSetupColumn(
                "##LoadDisplayUnitCombo",
                ImGuiTableColumnFlags_WidthFixed,
                loadUnitComboWidth);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Força");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(loadUnitComboWidth);
            if (ImGui::BeginCombo(
                    "##LoadForceDisplayUnit",
                    UnitConversion::GetForceUnitLabel(projectDocument.displayUnits.force)))
            {
                if (ImGui::Selectable("N", projectDocument.displayUnits.force == ForceUnit::Newton))
                {
                    updatedDisplayUnits.force = ForceUnit::Newton;
                    displayUnitsChanged = true;
                }
                if (ImGui::Selectable("kN", projectDocument.displayUnits.force == ForceUnit::Kilonewton))
                {
                    updatedDisplayUnits.force = ForceUnit::Kilonewton;
                    displayUnitsChanged = true;
                }
                ImGui::EndCombo();
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Momento");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(loadUnitComboWidth);
            if (ImGui::BeginCombo(
                    "##LoadMomentDisplayUnit",
                    UnitConversion::GetMomentUnitLabel(projectDocument.displayUnits.moment)))
            {
                if (ImGui::Selectable("N.m", projectDocument.displayUnits.moment == MomentUnit::NewtonMeter))
                {
                    updatedDisplayUnits.moment = MomentUnit::NewtonMeter;
                    displayUnitsChanged = true;
                }
                if (ImGui::Selectable(
                        "kN.m",
                        projectDocument.displayUnits.moment == MomentUnit::KilonewtonMeter))
                {
                    updatedDisplayUnits.moment = MomentUnit::KilonewtonMeter;
                    displayUnitsChanged = true;
                }
                ImGui::EndCombo();
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Distribuída");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(loadUnitComboWidth);
            if (ImGui::BeginCombo(
                    "##LoadDistributedDisplayUnit",
                    UnitConversion::GetDistributedLoadUnitLabel(projectDocument.displayUnits.distributedLoad)))
            {
                if (ImGui::Selectable(
                        "N/m",
                        projectDocument.displayUnits.distributedLoad == DistributedLoadUnit::NewtonPerMeter))
                {
                    updatedDisplayUnits.distributedLoad = DistributedLoadUnit::NewtonPerMeter;
                    displayUnitsChanged = true;
                }
                if (ImGui::Selectable(
                        "kN/m",
                        projectDocument.displayUnits.distributedLoad == DistributedLoadUnit::KilonewtonPerMeter))
                {
                    updatedDisplayUnits.distributedLoad = DistributedLoadUnit::KilonewtonPerMeter;
                    displayUnitsChanged = true;
                }
                ImGui::EndCombo();
            }

            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::EndGroup();

        if (displayUnitsChanged)
        {
            requests.displayUnitsUpdate.active = true;
            requests.displayUnitsUpdate.units = updatedDisplayUnits;
        }

        ToolbarSection::PushTitleToBottom("Visualização");
    }
    ToolbarSection::EndSection("Visualização");
    ToolbarSection::VerticalSeparator();

    requests.loadToolStateSync.active = true;
    requests.loadToolStateSync.setPendingNodalLoad = true;
    requests.loadToolStateSync.pendingNodalLoad = pendingNodalLoad;
    requests.loadToolStateSync.setPendingDistributedLoad = true;
    requests.loadToolStateSync.pendingDistributedLoad = pendingDistributedLoad;
    requests.loadToolStateSync.setPendingDistributedLoadVariable = true;
    requests.loadToolStateSync.pendingDistributedLoadVariable = pendingDistributedLoadVariable;
    requests.loadToolStateSync.setDistributedLoadPanelSelectionState = true;
    requests.loadToolStateSync.distributedLoadPanelSelectionToken = distributedLoadPanelSelectionToken;
    requests.loadToolStateSync.distributedLoadPanelSelectionTokenInitialized =
        distributedLoadPanelSelectionTokenInitialized;
}

void TopToolbar::DrawPropertiesTab(FrameRequests& requests, ProjectDocument& projectDocument, EditorState& editorState)
{
    const PropertySelectionOperations::BeamPropertySelectionInfo materialInfo =
        PropertySelectionOperations::ResolveMaterialSelectionInfo(
            projectDocument,
            editorState.selectionState.selection,
            editorState.currentMaterialId);

    const PropertySelectionOperations::BeamPropertySelectionInfo sectionInfo =
        PropertySelectionOperations::ResolveSectionSelectionInfo(
            projectDocument,
            editorState.selectionState.selection,
            editorState.currentSectionId);

    requests.propertyPanelStateSync.active = true;
    if (materialInfo.currentId != editorState.currentMaterialId)
    {
        requests.propertyPanelStateSync.setCurrentMaterialId = true;
        requests.propertyPanelStateSync.currentMaterialId = materialInfo.currentId;
    }

    if (sectionInfo.currentId != editorState.currentSectionId)
    {
        requests.propertyPanelStateSync.setCurrentSectionId = true;
        requests.propertyPanelStateSync.currentSectionId = sectionInfo.currentId;
    }

    ToolbarSection::BeginSection("MaterialSection", 320.0f);
    {
        auto queueMaterialSelection = [&](int materialId)
        {
            FrameRequests::BeamPropertySelectionEditRequest editRequest;
            editRequest.active = true;
            editRequest.setMaterial = true;
            editRequest.materialId = materialId;
            editRequest.beamIds.reserve(editorState.selectionState.selection.beamIds.size());
            for (int beamId : editorState.selectionState.selection.beamIds)
            {
                if (projectDocument.FindBeamById(beamId) != nullptr)
                {
                    editRequest.beamIds.push_back(beamId);
                }
            }
            requests.beamPropertyEdit = std::move(editRequest);
        };

        StructuralMaterial* currentMaterial = projectDocument.FindMaterialById(materialInfo.displayedId);
        const char* currentMaterialName = materialInfo.mixed
            ? "Vários"
            : (currentMaterial ? currentMaterial->name.c_str() : "Nenhum material");

        const ImVec2 defaultFramePadding = ImGui::GetStyle().FramePadding;
        const float materialExtraFramePaddingY = ToolbarSection::GetTitleVerticalOffset("Material") * 0.5f;
        ImGui::PushStyleVar(
            ImGuiStyleVar_FramePadding,
            ImVec2(defaultFramePadding.x, defaultFramePadding.y + materialExtraFramePaddingY));
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##MaterialCombo", currentMaterialName))
        {
            for (const StructuralMaterial& material : projectDocument.materials)
            {
                const bool isSelected = !materialInfo.mixed && (material.id == materialInfo.displayedId);

                if (ImGui::Selectable(material.name.c_str(), isSelected))
                {
                    queueMaterialSelection(material.id);
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
        ImGui::PopStyleVar();

        const ImVec2 sectionSpacing = ImGui::GetStyle().ItemSpacing;
        const float sectionAvailableWidth = ImGui::GetContentRegionAvail().x;
        const float sectionButtonWidth = (sectionAvailableWidth - 2.0f * sectionSpacing.x) / 3.0f;
        const float sectionButtonHeight =
            ToolbarSection::ClampFullHeightButtonHeight(ToolbarSection::GetButtonHeight("Material"));

        if (ImGui::Button("Criar", ImVec2(sectionButtonWidth, sectionButtonHeight)))
        {
            requests.dialog = ToolbarDialogRequest::OpenCreateMaterial;
        }

        ImGui::SameLine();

        if (ImGui::Button("Editar", ImVec2(sectionButtonWidth, sectionButtonHeight)))
        {
            requests.dialog = ToolbarDialogRequest::OpenEditMaterial;
        }

        ImGui::SameLine();

        const bool canRemoveMaterial =
            editorState.currentMaterialId >= 0 &&
            !projectDocument.IsMaterialUsed(editorState.currentMaterialId);

        if (!canRemoveMaterial)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Remover", ImVec2(sectionButtonWidth, sectionButtonHeight)))
        {
            requests.document = DocumentRequest::RemoveCurrentMaterial;
        }

        if (!canRemoveMaterial)
        {
            ImGui::EndDisabled();
        }
        ToolbarSection::PushTitleToBottom("Material");
    }
    ToolbarSection::EndSection("Material");

    ToolbarSection::VerticalSeparator();

    ToolbarSection::BeginSection("SectionSection", 320.0f);
    {
        auto queueSectionSelection = [&](int sectionId)
        {
            FrameRequests::BeamPropertySelectionEditRequest editRequest;
            editRequest.active = true;
            editRequest.setSection = true;
            editRequest.sectionId = sectionId;
            editRequest.beamIds.reserve(editorState.selectionState.selection.beamIds.size());
            for (int beamId : editorState.selectionState.selection.beamIds)
            {
                if (projectDocument.FindBeamById(beamId) != nullptr)
                {
                    editRequest.beamIds.push_back(beamId);
                }
            }
            requests.beamPropertyEdit = std::move(editRequest);
        };

        Section* currentSection = projectDocument.FindSectionById(sectionInfo.displayedId);
        const char* currentSectionName = sectionInfo.mixed
            ? "Várias"
            : (currentSection ? currentSection->name.c_str() : "Nenhuma seção");

        const ImVec2 defaultFramePadding = ImGui::GetStyle().FramePadding;
        const float sectionExtraFramePaddingY = ToolbarSection::GetTitleVerticalOffset("Seção") * 0.5f;
        ImGui::PushStyleVar(
            ImGuiStyleVar_FramePadding,
            ImVec2(defaultFramePadding.x, defaultFramePadding.y + sectionExtraFramePaddingY));
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##SectionCombo", currentSectionName))
        {
            for (const Section& section : projectDocument.sections)
            {
                const bool isSelected = !sectionInfo.mixed && (section.id == sectionInfo.displayedId);

                if (ImGui::Selectable(section.name.c_str(), isSelected))
                {
                    queueSectionSelection(section.id);
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
        ImGui::PopStyleVar();

        const ImVec2 sectionSpacing = ImGui::GetStyle().ItemSpacing;
        const float sectionAvailableWidth = ImGui::GetContentRegionAvail().x;
        const float sectionButtonWidth = (sectionAvailableWidth - 2.0f * sectionSpacing.x) / 3.0f;
        const float sectionButtonHeight =
            ToolbarSection::ClampFullHeightButtonHeight(ToolbarSection::GetButtonHeight("Seção"));

        if (ImGui::Button("Criar", ImVec2(sectionButtonWidth, sectionButtonHeight)))
        {
            requests.dialog = ToolbarDialogRequest::OpenCreateSection;
        }

        ImGui::SameLine();

        if (ImGui::Button("Editar", ImVec2(sectionButtonWidth, sectionButtonHeight)))
        {
            requests.dialog = ToolbarDialogRequest::OpenEditSection;
        }

        ImGui::SameLine();

        const bool canRemoveSection =
            editorState.currentSectionId >= 0 &&
            !projectDocument.IsSectionUsed(editorState.currentSectionId);

        if (!canRemoveSection)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Remover", ImVec2(sectionButtonWidth, sectionButtonHeight)))
        {
            requests.document = DocumentRequest::RemoveCurrentSection;
        }

        if (!canRemoveSection)
        {
            ImGui::EndDisabled();
        }
        ToolbarSection::PushTitleToBottom("Seção");
    }
    
    ToolbarSection::EndSection("Seção");
    ToolbarSection::VerticalSeparator();

    return;
}

void TopToolbar::DrawDiscretizationTab(FrameRequests& requests, ProjectDocument& projectDocument, EditorState& editorState)
{
    const auto& selectedBeamIds = editorState.selectionState.selection.beamIds;
    const bool hasSelection = !selectedBeamIds.empty();

    bool mixedMethod = false;
    bool mixedParts = false;
    bool mixedMaxDistance = false;
    bool mixedDistanceUnit = false;

    DiscretizationMethod commonMethod = editorState.discretization.method;
    int commonParts = editorState.discretization.parts;
    double commonMaxDistance = editorState.discretization.maxDistance;
    LengthUnit commonDistanceUnit = editorState.discretization.distanceUnit;

    if (hasSelection)
    {
        const Beam* firstBeam = projectDocument.FindBeamById(selectedBeamIds[0]);
        if (firstBeam)
        {
            commonMethod = firstBeam->discretizationMethod;
            commonParts = firstBeam->discretizationParts;
            commonMaxDistance = firstBeam->discretizationMaxDistance;
            commonDistanceUnit = firstBeam->discretizationDistanceUnit;

            for (size_t i = 1; i < selectedBeamIds.size(); ++i)
            {
                const Beam* beam = projectDocument.FindBeamById(selectedBeamIds[i]);
                if (!beam) continue;
                if (beam->discretizationMethod != commonMethod) mixedMethod = true;
                if (beam->discretizationParts != commonParts) mixedParts = true;
                if (beam->discretizationMaxDistance != commonMaxDistance) mixedMaxDistance = true;
                if (beam->discretizationDistanceUnit != commonDistanceUnit) mixedDistanceUnit = true;
            }
        }
    }

    auto emitEdit = [&](auto&& config) {
        FrameRequests::BeamPropertySelectionEditRequest editRequest;
        editRequest.active = true;
        editRequest.beamIds = selectedBeamIds;
        editRequest.discretizationMethod = commonMethod;
        editRequest.discretizationParts = commonParts;
        editRequest.discretizationMaxDistance = commonMaxDistance;
        editRequest.discretizationDistanceUnit = commonDistanceUnit;
        
        config(editRequest);
        requests.beamPropertyEdit = std::move(editRequest);
    };

    // --- Section 1: Propriedades ---
    ToolbarSection::BeginSection("DiscretizationMethod", 230.0f);
    {
        bool isByDivisions = !mixedMethod && (commonMethod == DiscretizationMethod::ByDivisions);
        if (ImGui::RadioButton(u8"Divisões", isByDivisions))
        {
            emitEdit([](auto& r) {
                r.setDiscretizationMethod = true;
                r.discretizationMethod = DiscretizationMethod::ByDivisions;
            });
        }

        ImGui::SameLine(110.0f);
        bool isByDistance = !mixedMethod && (commonMethod == DiscretizationMethod::ByDistance);
        if (ImGui::RadioButton(u8"Distância", isByDistance))
        {
            emitEdit([](auto& r) {
                r.setDiscretizationMethod = true;
                r.discretizationMethod = DiscretizationMethod::ByDistance;
            });
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

        if (mixedMethod)
        {
            ImGui::TextDisabled("(Múltiplos Métodos)");
        }
        else if (commonMethod == DiscretizationMethod::ByDivisions)
        {
            char buf[32];
            if (mixedParts) strcpy(buf, "");
            else sprintf(buf, "%d", commonParts);

            ImGui::SetNextItemWidth(60.0f);
            if (ImGui::InputText("##PartsInput", buf, sizeof(buf), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
            {
                int newParts = std::max(1, std::atoi(buf));
                emitEdit([&](auto& r) { r.setDiscretizationParts = true; r.discretizationParts = newParts; });
            }
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                int newParts = std::max(1, std::atoi(buf));
                emitEdit([&](auto& r) { r.setDiscretizationParts = true; r.discretizationParts = newParts; });
            }

            ImGui::SameLine();
            if (ImGui::Button("-", ImVec2(20, 20)))
            {
                int newVal = std::max(1, (mixedParts ? 10 : commonParts) - 1);
                emitEdit([&](auto& r) { r.setDiscretizationParts = true; r.discretizationParts = newVal; });
            }
            ImGui::SameLine(0, 2);
            if (ImGui::Button("+", ImVec2(20, 20)))
            {
                int newVal = std::min(10000, (mixedParts ? 10 : commonParts) + 1);
                emitEdit([&](auto& r) { r.setDiscretizationParts = true; r.discretizationParts = newVal; });
            }
            ImGui::SameLine();
            ImGui::TextUnformatted("partes");
        }
        else if (commonMethod == DiscretizationMethod::ByDistance)
        {
            char buf[64];
            if (mixedMaxDistance) strcpy(buf, "");
            else sprintf(buf, "%g", commonMaxDistance);

            ImGui::SetNextItemWidth(80.0f);
            if (ImGui::InputText("##DistanceInput", buf, sizeof(buf), ImGuiInputTextFlags_CharsScientific | ImGuiInputTextFlags_EnterReturnsTrue))
            {
                double newVal = std::atof(buf);
                emitEdit([&](auto& r) { r.setDiscretizationMaxDistance = true; r.discretizationMaxDistance = newVal; });
            }
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                double newVal = std::atof(buf);
                emitEdit([&](auto& r) { r.setDiscretizationMaxDistance = true; r.discretizationMaxDistance = newVal; });
            }

            ImGui::SameLine();
            const char* unitLabel = mixedDistanceUnit ? "Várias" : 
                (commonDistanceUnit == LengthUnit::Centimeter ? "cm" : 
                 (commonDistanceUnit == LengthUnit::Millimeter ? "mm" : "m"));

            ImGui::SetNextItemWidth(60.0f);
            if (ImGui::BeginCombo("##DistUnit", unitLabel))
            {
                auto updateUnit = [&](LengthUnit newUnit) {
                    double valueInMeters = UnitConversion::LengthToSI(commonMaxDistance, commonDistanceUnit);
                    double newValue = UnitConversion::LengthToDisplay(valueInMeters, newUnit);
                    emitEdit([&](auto& r) {
                        r.setDiscretizationDistanceUnit = true;
                        r.discretizationDistanceUnit = newUnit;
                        r.setDiscretizationMaxDistance = true;
                        r.discretizationMaxDistance = newValue;
                    });
                };
                if (ImGui::Selectable("m", !mixedDistanceUnit && commonDistanceUnit == LengthUnit::Meter)) updateUnit(LengthUnit::Meter);
                if (ImGui::Selectable("cm", !mixedDistanceUnit && commonDistanceUnit == LengthUnit::Centimeter)) updateUnit(LengthUnit::Centimeter);
                if (ImGui::Selectable("mm", !mixedDistanceUnit && commonDistanceUnit == LengthUnit::Millimeter)) updateUnit(LengthUnit::Millimeter);
                ImGui::EndCombo();
            }
        }

        ToolbarSection::PushTitleToBottom("Propriedades");
    }
    ToolbarSection::EndSection("Propriedades");
    ToolbarSection::VerticalSeparator();

    // --- Section 2: Visualização ---
    ToolbarSection::BeginSection("DiscretizationVis", 160.0f);
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        ImGui::Checkbox("Auto", &editorState.discretization.automatic);
        
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        ImGui::Checkbox("Ver malha", &editorState.discretization.show);
        
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        ImGui::Checkbox(u8"Ver pontos", &editorState.discretization.showNodes);

        ToolbarSection::PushTitleToBottom(u8"Visualização");
    }
    ToolbarSection::EndSection(u8"Visualização");
    ToolbarSection::VerticalSeparator();

    // --- Section 3: Ações ---
    ToolbarSection::DrawButtonSection(
        u8"Ações",
        {
            {"Discretizar", true, [&]() { requests.editor = EditorRequest::InvokeGenerateDiscretization; }, false},
            {"Limpar",      true, [&]() { requests.editor = EditorRequest::InvokeClearDiscretization; }, false}
        },
        buttonWidth,
        itemSpacingX);

    ToolbarSection::VerticalSeparator();
}

void TopToolbar::DrawImperfectionsTab(
    FrameRequests& requests,
    ProjectDocument& projectDocument,
    EditorState& editorState,
    const AnalysisState& analysisState)
{
    (void)projectDocument;

    auto emitSync = [&](auto&& fn) {
        requests.imperfectionSync.active = true;
        fn(requests.imperfectionSync);
    };

    // --- Section 1: Configurações ---
    ToolbarSection::BeginSection("ImperfectionProps", 380.0f);
    {
        if (ImGui::BeginTable("ImpPropTable", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX))
        {
            ImGui::TableSetupColumn("##Label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("##Input", ImGuiTableColumnFlags_WidthFixed, 280.0f);

            // Amplitude e0
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Amplitude e0");
            ImGui::TableSetColumnIndex(1);
            {
                ImGui::SetNextItemWidth(80.0f);
                double ampVal = UnitConversion::LengthToDisplay(editorState.imperfection.amplitude, editorState.imperfection.amplitudeUnit);
                if (ImGui::InputDouble("##AmpVal", &ampVal, 0.0, 0.0, "%.4f")) {
                    emitSync([&](auto& r) { r.setAmplitude = true; r.amplitude = ampVal; });
                }
                ImGui::SameLine(0, 8);
                ImGui::SetNextItemWidth(60.0f);
                if (ImGui::BeginCombo("##AmpUnit", UnitConversion::GetLengthUnitLabel(editorState.imperfection.amplitudeUnit))) {
                    const LengthUnit units[] = { LengthUnit::Meter, LengthUnit::Centimeter, LengthUnit::Millimeter };
                    for (auto u : units) {
                        if (ImGui::Selectable(UnitConversion::GetLengthUnitLabel(u), editorState.imperfection.amplitudeUnit == u)) {
                            emitSync([&](auto& r) { r.setAmplitudeUnit = true; r.amplitudeUnit = u; });
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            // Modos
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Modo");
            ImGui::TableSetColumnIndex(1);
            {
                ImGui::SetNextItemWidth(240.0f);
                const char* modePreview = "Nenhum";
                if (!editorState.imperfection.availableModes.empty()) {
                    if (editorState.imperfection.selectedModeIndex > 0 && editorState.imperfection.selectedModeIndex <= (int)editorState.imperfection.availableModes.size()) {
                        static char modeBuf[48];
                        const auto& m = editorState.imperfection.availableModes[editorState.imperfection.selectedModeIndex - 1];
                        sprintf(modeBuf, "Modo %d (\xCE\xBB=%.2f)", m.id, m.lambda);
                        modePreview = modeBuf;
                    }
                }

                if (ImGui::BeginCombo("##SelectedMode", modePreview))
                {
                    if (ImGui::Selectable("Nenhum", editorState.imperfection.selectedModeIndex == 0)) {
                        editorState.imperfection.selectedModeIndex = 0;
                        emitSync([](auto& r) { r.setModeIndex = true; r.modeIndex = 0; });
                    }
                    for (const auto& mode : editorState.imperfection.availableModes) {  
                        char modeLabel[64];
                        sprintf(modeLabel, "Modo %d (\xCE\xBB=%.3f)", mode.id, mode.lambda);
                        if (ImGui::Selectable(modeLabel, editorState.imperfection.selectedModeIndex == mode.id)) {
                            editorState.imperfection.selectedModeIndex = mode.id;
                            emitSync([&](auto& r) { r.setModeIndex = true; r.modeIndex = mode.id; });
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            // Quantidade de Modos
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Qtde. Modos");
            ImGui::TableSetColumnIndex(1);
            {
                ImGui::SetNextItemWidth(60.0f);
                static int uiMaxModes = 5;
                if (!ImGui::IsItemActive()) uiMaxModes = editorState.imperfection.maxModes;
                if (ImGui::InputInt("##MaxModes", &uiMaxModes, 0, 0)) {
                    if (uiMaxModes < 1) uiMaxModes = 1;
                    if (uiMaxModes > 50) uiMaxModes = 50;
                    emitSync([&](auto& r) { r.setMaxModes = true; r.maxModes = uiMaxModes; });
                }
            }

            ImGui::EndTable();
        }
        ToolbarSection::PushTitleToBottom(u8"Configurações");
    }
    ToolbarSection::EndSection(u8"Configurações");
    ToolbarSection::VerticalSeparator();

    // --- Section 2: Visualização ---
    ToolbarSection::BeginSection("ImperfectionVis", 110.0f);
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        if (ImGui::Checkbox("Aplicar", &editorState.imperfection.enabled))
        {
            emitSync([&](auto& r) {
                r.setEnabled = true;
                r.enabled = editorState.imperfection.enabled;
            });
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        if (ImGui::Checkbox(u8"Ver Prévia", &editorState.imperfection.show))
        {
            emitSync([&](auto& r) {
                r.setShow = true;
                r.show = editorState.imperfection.show;
            });
        }
        ToolbarSection::PushTitleToBottom(u8"Visualização");
    }
    ToolbarSection::EndSection(u8"Visualização");
    ToolbarSection::VerticalSeparator();

    // --- Section 3: Ações ---
    ToolbarSection::DrawButtonSection(
        u8"Ações",
        {
            {"Calcular", true, [&]() { requests.editor = EditorRequest::InvokeCalculateBucklingModes; }, false},
            {"Atualizar", editorState.imperfection.enabled, [&]() { requests.editor = EditorRequest::InvokeApplyImperfections; }, false},
            {"Limpar",   true, [&]() { requests.editor = EditorRequest::InvokeClearImperfections; }, false}
        },
        buttonWidth,
        itemSpacingX);

    ToolbarSection::VerticalSeparator();
}

void TopToolbar::DrawAnalysisTab(
    FrameRequests& requests,
    ProjectDocument& projectDocument,
    EditorState& editorState,
    const AnalysisState& analysisState)
{
    (void)projectDocument;
    (void)editorState;
    
    // --- Section 1: Parâmetros ---
    ToolbarSection::BeginSection("AnalysisParams", 340.0f);
    {
        if (ImGui::BeginTable("AnalysisConfigTable", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX))
        {
            ImGui::TableSetupColumn("##Labels", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("##Inputs", ImGuiTableColumnFlags_WidthFixed, 250.0f);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Tipo");

            ImGui::TableSetColumnIndex(1);
            const char* analysisTypes[] = { "Estática Linear", "Não-Linear (N-R)", "Não-Linear (Comp. Arco)" };
            int currentType = static_cast<int>(analysisState.options.type);
            ImGui::SetNextItemWidth(180.0f);
            if (ImGui::Combo("##AnalysisType", &currentType, analysisTypes, IM_ARRAYSIZE(analysisTypes)))
            {
                requests.analysisSettings.active = true;
                requests.analysisSettings.setType = true;
                requests.analysisSettings.type = static_cast<AnalysisType>(currentType);

                // Sugerir mais passos para Arc Length se o valor atual for baixo
                if (requests.analysisSettings.type == AnalysisType::NonLinearStaticArcLength &&
                    analysisState.options.numSteps < 100)
                {
                    requests.analysisSettings.setNonLinearParams = true;
                    requests.analysisSettings.numSteps = 100;
                }
            }

            if (analysisState.options.type == AnalysisType::NonLinearStaticNR || 
                analysisState.options.type == AnalysisType::NonLinearStaticArcLength)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("NL Params");

                ImGui::TableSetColumnIndex(1);
                ImGui::BeginGroup();

                ImGui::TextUnformatted("Passos");
                ImGui::SameLine(50.0f);
                ImGui::SetNextItemWidth(50.0f);
                int steps = analysisState.options.numSteps;
                if (ImGui::InputInt("##Steps", &steps, 0, 0))
                {
                    if (steps < 1) steps = 1;
                    requests.analysisSettings.active = true;
                    requests.analysisSettings.setNonLinearParams = true;
                    requests.analysisSettings.numSteps = steps;
                }

                ImGui::SameLine(115.0f);
                ImGui::TextUnformatted("Iter. Máx");
                ImGui::SameLine(175.0f);
                ImGui::SetNextItemWidth(50.0f);
                int maxIter = analysisState.options.maxIter;
                if (ImGui::InputInt("##MaxIter", &maxIter, 0, 0))
                {
                    if (maxIter < 1) maxIter = 1;
                    requests.analysisSettings.active = true;
                    requests.analysisSettings.setNonLinearParams = true;
                    requests.analysisSettings.maxIter = maxIter;
                }

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
                ImGui::TextUnformatted("Toler.");
                ImGui::SameLine(50.0f);
                ImGui::SetNextItemWidth(90.0f);
                double tol = analysisState.options.tolerance;
                if (ImGui::InputDouble("##Tolerance", &tol, 0.0, 0.0, "%.1e"))
                {
                    requests.analysisSettings.active = true;
                    requests.analysisSettings.setNonLinearParams = true;
                    requests.analysisSettings.tolerance = tol;
                }
                ImGui::EndGroup();
            }

            ImGui::EndTable();
        }

        ToolbarSection::PushTitleToBottom("Parâmetros");
    }
    ToolbarSection::EndSection("Parâmetros");
    ToolbarSection::VerticalSeparator();

    // --- Section 2: Execução ---
    ToolbarSection::DrawButtonSection(
        "Execução",
        {
            {"Executar", true, [&]() { requests.analysisExecution.run = true; }, false},
            {"Limpar",   true, [&]() { requests.analysisExecution.clear = true; }, false},
        },
        buttonWidth,
        itemSpacingX);

    ToolbarSection::VerticalSeparator();
}

void TopToolbar::DrawResultsTab(
    FrameRequests& requests,
    ProjectDocument& projectDocument,
    EditorState& editorState,
    const AnalysisState& analysisState)
{
    (void)projectDocument;
    (void)editorState;
    
    requests.analysisView.active = true;

    // --- Section 1: Visibilidade ---
    ToolbarSection::BeginSection("ResultsVisibility", 160.0f);
    {
        bool showDeformed = analysisState.showDeformedShape;
        if (ImGui::Checkbox("Deformada", &showDeformed))
        {
            requests.analysisView.setDeformedShape = true;
            requests.analysisView.showDeformedShape = showDeformed;
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        bool showReactions = analysisState.showReactions;
        if (ImGui::Checkbox(u8"Reações", &showReactions))
        {
            requests.analysisView.setReactions = true;
            requests.analysisView.showReactions = showReactions;
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        bool showCurve = analysisState.showEquilibriumCurve;
        if (ImGui::Checkbox(u8"Curva Equilíbrio", &showCurve))
        {
            requests.analysisView.setEquilibriumCurve = true;
            requests.analysisView.showEquilibriumCurve = showCurve;
        }

        ToolbarSection::PushTitleToBottom(u8"Visibilidade");
    }
    ToolbarSection::EndSection(u8"Visibilidade");
    ToolbarSection::VerticalSeparator();

    // --- Section 2: Escalas ---
    ToolbarSection::BeginSection("ResultsScales", 260.0f);
    {
        const float sliderWidth = 22.0f;
        const float sliderSpacing = 8.0f;
        const float groupSpacing = 10.0f;
        const float limitInputWidth = 44.0f;
        const float sliderLimitMaximum = 300.0f;
        const float inlineGap = 2.0f;
        const float sliderHeight = 70.0f;

        auto processSliderInteraction = [&](float& val, float minVal, float limit, auto triggerUpdate) {
            if (ImGui::IsItemHovered()) {
                float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0.0f) {
                    val += wheel * (limit * 0.05f); // 5% step per scroll click
                    val = std::clamp(val, minVal, limit);
                    triggerUpdate(val);
                }
                
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                    val += (limit * 0.05f);
                    val = std::clamp(val, minVal, limit);
                    triggerUpdate(val);
                }
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                    val -= (limit * 0.05f);
                    val = std::clamp(val, minVal, limit);
                    triggerUpdate(val);
                }
            }
        };

        ImGui::BeginGroup();
        {
            // Force Slider
            ImGui::BeginGroup();
            {
                const float startX = ImGui::GetCursorPosX();
                float fMax = analysisState.reactionForceMaxPixels;
                if (ImGui::VSliderFloat("##ReactFSlider", ImVec2(sliderWidth, sliderHeight), &fMax, 1.0f, analysisState.reactionForceSliderLimitPixels, ""))
                {
                    requests.analysisView.setReactionForceMaxPixels = true;
                    requests.analysisView.reactionForceMaxPixels = fMax;
                }
                processSliderInteraction(fMax, 1.0f, analysisState.reactionForceSliderLimitPixels, [&](float v) {
                    requests.analysisView.setReactionForceMaxPixels = true;
                    requests.analysisView.reactionForceMaxPixels = v;
                });

                const float labelW = ImGui::CalcTextSize("F").x;
                ImGui::SetCursorPosX(startX + (sliderWidth - labelW) * 0.5f);
                ImGui::TextUnformatted("F");
            }
            ImGui::EndGroup();

            ImGui::SameLine(0.0f, sliderSpacing);

            // Moment Slider
            ImGui::BeginGroup();
            {
                const float startX = ImGui::GetCursorPosX();
                float mMax = analysisState.reactionMomentMaxPixels;
                if (ImGui::VSliderFloat("##ReactMSlider", ImVec2(sliderWidth, sliderHeight), &mMax, 1.0f, analysisState.reactionMomentSliderLimitPixels, ""))
                {
                    requests.analysisView.setReactionMomentMaxPixels = true;
                    requests.analysisView.reactionMomentMaxPixels = mMax;
                }
                processSliderInteraction(mMax, 1.0f, analysisState.reactionMomentSliderLimitPixels, [&](float v) {
                    requests.analysisView.setReactionMomentMaxPixels = true;
                    requests.analysisView.reactionMomentMaxPixels = v;
                });

                const float labelW = ImGui::CalcTextSize("M").x;
                ImGui::SetCursorPosX(startX + (sliderWidth - labelW) * 0.5f);
                ImGui::TextUnformatted("M");
            }
            ImGui::EndGroup();

            ImGui::SameLine(0.0f, sliderSpacing);

            // Diagram Scale Slider
            ImGui::BeginGroup();
            {
                const float startX = ImGui::GetCursorPosX();
                float qMax = analysisState.diagramMaxPixels;
                if (ImGui::VSliderFloat("##DiagSlider", ImVec2(sliderWidth, sliderHeight), &qMax, 1.0f, analysisState.diagramSliderLimitPixels, ""))
                {
                    requests.analysisView.setDiagramMaxPixels = true;
                    requests.analysisView.diagramMaxPixels = qMax;
                }
                processSliderInteraction(qMax, 1.0f, analysisState.diagramSliderLimitPixels, [&](float v) {
                    requests.analysisView.setDiagramMaxPixels = true;
                    requests.analysisView.diagramMaxPixels = v;
                });

                const float labelW = ImGui::CalcTextSize("Diag").x;
                ImGui::SetCursorPosX(startX + (sliderWidth - labelW) * 0.5f);
                ImGui::TextUnformatted("Diag");
            }
            ImGui::EndGroup();

            ImGui::SameLine(0.0f, sliderSpacing);

            // Deformation Scale Slider
            ImGui::BeginGroup();
            {
                const float startX = ImGui::GetCursorPosX();
                float dMax = analysisState.deformationScale;
                if (ImGui::VSliderFloat("##DeformSlider", ImVec2(sliderWidth, sliderHeight), &dMax, 1.0f, analysisState.deformationSliderLimitPixels, ""))
                {
                    requests.analysisView.setDeformationScale = true;
                    requests.analysisView.deformationScale = dMax;
                }
                processSliderInteraction(dMax, 1.0f, analysisState.deformationSliderLimitPixels, [&](float v) {
                    requests.analysisView.setDeformationScale = true;
                    requests.analysisView.deformationScale = v;
                });

                const float labelW = ImGui::CalcTextSize("Def").x;
                ImGui::SetCursorPosX(startX + (sliderWidth - labelW) * 0.5f);
                ImGui::TextUnformatted("Def");
            }
            ImGui::EndGroup();
        }
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, groupSpacing);

        // Limit Inputs
        ImGui::BeginGroup();
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 1.0f));
        if (ImGui::BeginTable("ResultsLimitTable", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX))
        {
            ImGui::TableSetupColumn("##L", ImGuiTableColumnFlags_WidthFixed, 36.0f);
            ImGui::TableSetupColumn("##I", ImGuiTableColumnFlags_WidthFixed, limitInputWidth + inlineGap);
            ImGui::TableSetupColumn("##U", ImGuiTableColumnFlags_WidthFixed, 25.0f);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Fmáx");
            ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(limitInputWidth);
            float fLimit = analysisState.reactionForceSliderLimitPixels;
            if (ImGui::InputFloat("##RLimF", &fLimit, 0, 0, "%.0f")) {
                fLimit = std::clamp(fLimit, 1.0f, sliderLimitMaximum);
                requests.analysisView.setReactionForceSliderLimitPixels = true;
                requests.analysisView.reactionForceSliderLimitPixels = fLimit;
            }
            ImGui::TableSetColumnIndex(2); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("px");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Mmáx");
            ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(limitInputWidth);
            float mLimit = analysisState.reactionMomentSliderLimitPixels;
            if (ImGui::InputFloat("##RLimM", &mLimit, 0, 0, "%.0f")) {
                mLimit = std::clamp(mLimit, 1.0f, sliderLimitMaximum);
                requests.analysisView.setReactionMomentSliderLimitPixels = true;
                requests.analysisView.reactionMomentSliderLimitPixels = mLimit;
            }
            ImGui::TableSetColumnIndex(2); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("px");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Qmáx");
            ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(limitInputWidth);
            float qLimit = analysisState.diagramSliderLimitPixels;
            if (ImGui::InputFloat("##RLimQ", &qLimit, 0, 0, "%.0f")) {
                qLimit = std::clamp(qLimit, 1.0f, sliderLimitMaximum);
                requests.analysisView.setDiagramSliderLimitPixels = true;
                requests.analysisView.diagramSliderLimitPixels = qLimit;
            }
            ImGui::TableSetColumnIndex(2); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("px");

            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::EndGroup();

        ToolbarSection::PushTitleToBottom(u8"Escalas");
    }
    ToolbarSection::EndSection(u8"Escalas");
    ToolbarSection::VerticalSeparator();

    // --- Section 3: Diagramas ---
    ToolbarSection::BeginSection("ResultsDiagrams", 480.0f);
    {
        const float localButtonWidth = 84.0f;
        const float buttonHeight = ToolbarSection::ClampFullHeightButtonHeight(ToolbarSection::GetButtonHeight("Diagramas"));
        
        ImGui::BeginGroup();
        ToolbarSection::DrawButton({ "Nenhum", true, [&]() {
            requests.analysisView.setDiagramMode = true;
            requests.analysisView.diagramMode = AnalysisDiagramMode::None;
        }, analysisState.diagramMode == AnalysisDiagramMode::None }, ImVec2(localButtonWidth, buttonHeight));
        
        ImGui::SameLine(0, 4);
        ToolbarSection::DrawButton({ "Axial", true, [&]() {
            requests.analysisView.setDiagramMode = true;
            requests.analysisView.diagramMode = AnalysisDiagramMode::AxialForce;
        }, analysisState.diagramMode == AnalysisDiagramMode::AxialForce }, ImVec2(localButtonWidth, buttonHeight));
        
        ImGui::SameLine(0, 4);
        ToolbarSection::DrawButton({ "Cortante", true, [&]() {
            requests.analysisView.setDiagramMode = true;
            requests.analysisView.diagramMode = AnalysisDiagramMode::ShearForce;
        }, analysisState.diagramMode == AnalysisDiagramMode::ShearForce }, ImVec2(localButtonWidth, buttonHeight));
        
        ImGui::SameLine(0, 4);
        ToolbarSection::DrawButton({ "Momento", true, [&]() {
            requests.analysisView.setDiagramMode = true;
            requests.analysisView.diagramMode = AnalysisDiagramMode::BendingMoment;
        }, analysisState.diagramMode == AnalysisDiagramMode::BendingMoment }, ImVec2(localButtonWidth, buttonHeight));
        ImGui::EndGroup();

        ImGui::SameLine(0, 15);
        ImGui::BeginGroup();
        bool showValues = analysisState.showValues;
        if (ImGui::Checkbox("Valores", &showValues))
        {
            requests.analysisView.setShowValues = true;
            requests.analysisView.showValues = showValues;
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        bool showAll = analysisState.showAllDiscretizationValues;
        if (ImGui::Checkbox("Completos", &showAll))
        {
            requests.analysisView.setShowAllDiscretizationValues = true;
            requests.analysisView.showAllDiscretizationValues = showAll;
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        bool showUnits = analysisState.showUnits;
        if (ImGui::Checkbox("Unidades", &showUnits))
        {
            requests.analysisView.setShowUnits = true;
            requests.analysisView.showUnits = showUnits;
        }
        ImGui::EndGroup();


        ToolbarSection::PushTitleToBottom(u8"Diagramas");
    }
    ToolbarSection::EndSection(u8"Diagramas");
    ToolbarSection::VerticalSeparator();

    // --- Section 4: Histórico ---
    ToolbarSection::BeginSection("ResultsHistory", 280.0f);
    {
        if (analysisState.hasResults && analysisState.history.size() > 1)
        {
            ImGui::TextUnformatted("Passo de Carga");
            int step = analysisState.currentStepIndex;
            ImGui::SetNextItemWidth(260.0f);
            if (ImGui::SliderInt("##LoadStep", &step, 0, static_cast<int>(analysisState.history.size()) - 1))
            {
                requests.analysisView.setCurrentStepIndex = true;
                requests.analysisView.currentStepIndex = step;
            }
            
            if (step >= 0 && step < static_cast<int>(analysisState.history.size())) {
                ImGui::Text(u8"λ: %.4f", analysisState.history[step].loadFactor);
            }
        }
        else
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f);
            ImGui::TextDisabled("Sem histórico disponível");
        }

        ToolbarSection::PushTitleToBottom(u8"Histórico");
    }
    ToolbarSection::EndSection(u8"Histórico");
    ToolbarSection::VerticalSeparator();
}

void TopToolbar::DrawAboutTab()
{
    // Lazy load the logo texture
    static Texture2D logoTexture{};
    static bool logoLoaded = false;
    if (!logoLoaded)
    {
        logoTexture = LoadTexture("resources/logo.png");
        if (logoTexture.id > 0)
        {
            SetTextureFilter(logoTexture, TEXTURE_FILTER_BILINEAR);
        }
        logoLoaded = true;
    }

    // --- Seção 1: Aplicativo ---
    ToolbarSection::BeginSection("AboutAppSection", 200.0f);
    {
        // Alinhamento vertical centralizado para o logo e 3 linhas
        ImGui::SetCursorPosY(20.0f);
        
        ImGui::BeginGroup();
        if (logoTexture.id > 0)
        {
            rlImGuiImageSize(&logoTexture, 48, 48); // Logo mais proporcional
        }
        else
        {
            ImGui::Dummy(ImVec2(48, 48));
        }
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, 12.0f);

        ImGui::BeginGroup();
        ImGui::Text(u8"NLtool");
        ImGui::TextDisabled(u8"Análise Estrutural");
        ImGui::TextDisabled(u8"Não Linear de Barras");
        ImGui::EndGroup();

        ToolbarSection::PushTitleToBottom(u8"Aplicativo");
    }
    ToolbarSection::EndSection(u8"Aplicativo");

    ToolbarSection::VerticalSeparator();

    // --- Seção 2: Autores ---
    ToolbarSection::BeginSection("AboutAuthorsSection", 418.0f);
    {
        // Alinhamento vertical centralizado (tabela de 3 linhas ~50px -> offset de 20px)
        ImGui::SetCursorPosY(20.0f);
        
        if (ImGui::BeginTable("AuthorsTable", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX))
        {
            ImGui::TableSetupColumn("##ColName", ImGuiTableColumnFlags_WidthFixed, 265.0f);
            ImGui::TableSetupColumn("##ColEmail", ImGuiTableColumnFlags_WidthFixed, 145.0f);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(u8"Dr. Eduardo Vicente Wolf Trentini (Orientador)");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextDisabled("etrentini@ufu.br");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(u8"Arthur Cunha Pena");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextDisabled("arthur.cunha.pena@ufu.br");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(u8"Pedro Oscar Bertholucci Aguiar");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextDisabled("pedro.oscar@ufu.br");

            ImGui::EndTable();
        }

        ToolbarSection::PushTitleToBottom(u8"Autores");
    }
    ToolbarSection::EndSection(u8"Autores");
    ToolbarSection::VerticalSeparator();
}
