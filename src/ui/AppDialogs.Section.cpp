#include "ui/AppDialogs.h"

#include <algorithm>
#include <cstdio>

#include "imgui.h"
#include "utils/UnitConversion.h"

namespace
{
    constexpr const char* sectionDialogPopupId = u8"Seção###SectionDialog";
}

void AppDialogs::BeginCreateSection(const ProjectDocument& document)
{
    sectionDialogMode = DialogMode::Create;
    closeSectionDialogRequested = false;

    sectionName = "Nova seção " + std::to_string(document.nextSectionId);

    sectionArea = 800.0;
    sectionInertia = 106666.666666;
    sectionAreaLastValid = sectionArea;
    sectionInertiaLastValid = sectionInertia;
}

void AppDialogs::BeginEditSection(const ProjectDocument& document, const EditorState& state)
{
    const Section* section = document.FindSectionById(state.currentSectionId);
    if (section == nullptr)
    {
        sectionDialogMode = DialogMode::None;
        closeSectionDialogRequested = false;
        return;
    }

    sectionDialogMode = DialogMode::Edit;
    closeSectionDialogRequested = false;

    sectionName = section->name;

    sectionArea = UnitConversion::AreaToDisplay(
        section->area,
        document.displayUnits.area);

    sectionInertia = UnitConversion::InertiaToDisplay(
        section->inertia,
        document.displayUnits.inertia);
    sectionAreaLastValid = sectionArea;
    sectionInertiaLastValid = sectionInertia;
}

SectionDialogResult AppDialogs::DrawSectionDialog(const ProjectDocument& document, const EditorState& state)
{
    SectionDialogResult result{};

    if (closeSectionDialogRequested && !ImGui::IsPopupOpen(sectionDialogPopupId))
    {
        sectionDialogMode = DialogMode::None;
        closeSectionDialogRequested = false;
    }

    const char* areaUnit =
        UnitConversion::GetAreaUnitLabel(document.displayUnits.area);

    const char* inertiaUnit =
        UnitConversion::GetInertiaUnitLabel(document.displayUnits.inertia);

    const float labelWidth = ComputeMaxTextWidth(
        {
            "Nome",
            "Área (A)",
            "Momento de inércia (Iz)"
        }) + 2.0f;

    const float unitWidth = ComputeMaxTextWidth(
        {
            "",
            areaUnit,
            inertiaUnit
        }) + 2.0f;

    const char* sectionWarningText = "A e Iz devem ser maiores que zero.";
    const float sectionBaseDialogWidth = ComputeDialogWidth(labelWidth, dialogInputWidth, unitWidth);
    const float sectionWarningWidth =
        dialogWindowPaddingX * 2.0f + ImGui::CalcTextSize(sectionWarningText).x;

    ImGui::SetNextWindowSize(
        ImVec2(std::max(sectionBaseDialogWidth, sectionWarningWidth), 0.0f),
        ImGuiCond_Appearing);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(dialogWindowPaddingX, dialogWindowPaddingY));

    if (ImGui::BeginPopupModal(sectionDialogPopupId, nullptr, ImGuiWindowFlags_NoResize))
    {
        if (closeSectionDialogRequested)
        {
            sectionDialogMode = DialogMode::None;
            closeSectionDialogRequested = false;
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            ImGui::PopStyleVar();
            return result;
        }

        if (ImGui::BeginTable("SectionNameTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("RótuloNome", ImGuiTableColumnFlags_WidthFixed, labelWidth);
            ImGui::TableSetupColumn(
                "ValorNome",
                ImGuiTableColumnFlags_WidthFixed,
                dialogInputWidth + unitWidth + ImGui::GetStyle().ItemSpacing.x);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Nome");

            ImGui::TableSetColumnIndex(1);
            char nameBuf[128];
            std::snprintf(nameBuf, sizeof(nameBuf), "%s", sectionName.c_str());
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputText("##SectionName", nameBuf, sizeof(nameBuf)))
            {
                sectionName = nameBuf;
            }

            ImGui::EndTable();
        }

        if (BeginPropertyTable("SectionPropertyTable", labelWidth, dialogInputWidth, unitWidth))
        {
            DrawPositiveDoubleInputRow(
                "Área (A)",
                "##SectionArea",
                sectionArea,
                sectionAreaLastValid,
                areaUnit,
                "%.6g");

            DrawPositiveDoubleInputRow(
                "Momento de inércia (Iz)",
                "##SectionInertia",
                sectionInertia,
                sectionInertiaLastValid,
                inertiaUnit,
                "%.6g");

            ImGui::EndTable();
        }

        ImGui::Spacing();
        const bool hasValidSectionValues =
            sectionArea > 0.0 &&
            sectionInertia > 0.0;
        const ImVec2 sectionWarningSize = ImGui::CalcTextSize(sectionWarningText);

        if (!hasValidSectionValues)
        {
            ImGui::TextColored(
                ImVec4(0.75f, 0.18f, 0.18f, 1.0f),
                sectionWarningText);
        }
        else
        {
            ImGui::Dummy(ImVec2(0.0f, sectionWarningSize.y));
        }

        DrawDialogButtons();

        if (!hasValidSectionValues)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Salvar", ImVec2(dialogButtonWidth, 0.0f)))
        {
            const double areaInSI = UnitConversion::AreaToSI(
                sectionArea,
                document.displayUnits.area);

            const double inertiaInSI = UnitConversion::InertiaToSI(
                sectionInertia,
                document.displayUnits.inertia);

            if (sectionDialogMode == DialogMode::Create)
            {
                result.action = SectionDialogResult::Action::Create;
                result.targetSectionId = -1;
            }
            else if (sectionDialogMode == DialogMode::Edit)
            {
                result.action = SectionDialogResult::Action::Update;
                result.targetSectionId = state.currentSectionId;
            }

            result.name = sectionName;
            result.area = areaInSI;
            result.inertia = inertiaInSI;

            sectionDialogMode = DialogMode::None;
            closeSectionDialogRequested = false;
            ImGui::CloseCurrentPopup();
        }

        if (!hasValidSectionValues)
        {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancelar", ImVec2(dialogButtonWidth, 0.0f)))
        {
            sectionDialogMode = DialogMode::None;
            closeSectionDialogRequested = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
    return result;
}
