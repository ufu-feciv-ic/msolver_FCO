#include "ui/AppDialogs.h"

#include <algorithm>
#include <cstdio>

#include "imgui.h"
#include "utils/UnitConversion.h"

namespace
{
    constexpr const char* materialDialogPopupId = u8"Material###MaterialDialog";
}

void AppDialogs::BeginCreateMaterial(const ProjectDocument& document)
{
    materialDialogMode = DialogMode::Create;
    closeMaterialDialogRequested = false;

    materialName = "Novo material " + std::to_string(document.nextMaterialId);

    materialYoungModulus = 30.0;
    materialThermalExpansion = 1.0e-5;
    materialYoungModulusLastValid = materialYoungModulus;
    materialThermalExpansionLastValid = materialThermalExpansion;
}

void AppDialogs::BeginEditMaterial(const ProjectDocument& document, const EditorState& state)
{
    const StructuralMaterial* material = document.FindMaterialById(state.currentMaterialId);
    if (material == nullptr)
    {
        materialDialogMode = DialogMode::None;
        closeMaterialDialogRequested = false;
        return;
    }

    materialDialogMode = DialogMode::Edit;
    closeMaterialDialogRequested = false;

    materialName = material->name;

    materialYoungModulus = UnitConversion::ElasticModulusToDisplay(
        material->youngModulus,
        document.displayUnits.elasticModulus);

    materialThermalExpansion = material->thermalExpansion;
    materialYoungModulusLastValid = materialYoungModulus;
    materialThermalExpansionLastValid = materialThermalExpansion;
}

MaterialDialogResult AppDialogs::DrawMaterialDialog(const ProjectDocument& document, const EditorState& state)
{
    MaterialDialogResult result{};

    if (closeMaterialDialogRequested && !ImGui::IsPopupOpen(materialDialogPopupId))
    {
        materialDialogMode = DialogMode::None;
        closeMaterialDialogRequested = false;
    }

    const char* modulusUnit =
        UnitConversion::GetElasticModulusUnitLabel(document.displayUnits.elasticModulus);

    const float labelWidth = ComputeMaxTextWidth(
        {
            "Nome",
            "Módulo de elasticidade (E)",
            "Coeficiente de dilatação térmica (α)"
        }) + 2.0f;

    const float unitWidth = ComputeMaxTextWidth(
        {
            "",
            modulusUnit,
            "1/°C"
        }) + 2.0f;

    const char* materialWarningText = "E e α devem ser maiores que zero.";
    const float materialBaseDialogWidth = ComputeDialogWidth(labelWidth, dialogInputWidth, unitWidth);
    const float materialWarningWidth =
        dialogWindowPaddingX * 2.0f + ImGui::CalcTextSize(materialWarningText).x;

    ImGui::SetNextWindowSize(
        ImVec2(std::max(materialBaseDialogWidth, materialWarningWidth), 0.0f),
        ImGuiCond_Appearing);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(dialogWindowPaddingX, dialogWindowPaddingY));

    if (ImGui::BeginPopupModal(materialDialogPopupId, nullptr, ImGuiWindowFlags_NoResize))
    {
        if (closeMaterialDialogRequested)
        {
            materialDialogMode = DialogMode::None;
            closeMaterialDialogRequested = false;
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            ImGui::PopStyleVar();
            return result;
        }

        if (ImGui::BeginTable("MaterialNameTable", 2, ImGuiTableFlags_SizingFixedFit))
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
            std::snprintf(nameBuf, sizeof(nameBuf), "%s", materialName.c_str());
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputText("##MaterialName", nameBuf, sizeof(nameBuf)))
            {
                materialName = nameBuf;
            }

            ImGui::EndTable();
        }

        if (BeginPropertyTable("MaterialPropertyTable", labelWidth, dialogInputWidth, unitWidth))
        {
            DrawPositiveDoubleInputRow(
                "Módulo de elasticidade (E)",
                "##MaterialE",
                materialYoungModulus,
                materialYoungModulusLastValid,
                modulusUnit,
                "%.6g");

            DrawPositiveDoubleInputRow(
                "Coeficiente de dilatação térmica (α)",
                "##MaterialAlpha",
                materialThermalExpansion,
                materialThermalExpansionLastValid,
                "1/°C",
                "%.6g");

            ImGui::EndTable();
        }

        ImGui::Spacing();
        const bool hasValidMaterialValues =
            materialYoungModulus > 0.0 &&
            materialThermalExpansion > 0.0;
        const ImVec2 materialWarningSize = ImGui::CalcTextSize(materialWarningText);

        if (!hasValidMaterialValues)
        {
            ImGui::TextColored(
                ImVec4(0.75f, 0.18f, 0.18f, 1.0f),
                materialWarningText);
        }
        else
        {
            ImGui::Dummy(ImVec2(0.0f, materialWarningSize.y));
        }

        DrawDialogButtons();

        if (!hasValidMaterialValues)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Salvar", ImVec2(dialogButtonWidth, 0.0f)))
        {
            const double youngModulusInSI = UnitConversion::ElasticModulusToSI(
                materialYoungModulus,
                document.displayUnits.elasticModulus);

            if (materialDialogMode == DialogMode::Create)
            {
                result.action = MaterialDialogResult::Action::Create;
                result.targetMaterialId = -1;
            }
            else if (materialDialogMode == DialogMode::Edit)
            {
                result.action = MaterialDialogResult::Action::Update;
                result.targetMaterialId = state.currentMaterialId;
            }

            result.name = materialName;
            result.youngModulus = youngModulusInSI;
            result.thermalExpansion = materialThermalExpansion;

            materialDialogMode = DialogMode::None;
            closeMaterialDialogRequested = false;
            ImGui::CloseCurrentPopup();
        }

        if (!hasValidMaterialValues)
        {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancelar", ImVec2(dialogButtonWidth, 0.0f)))
        {
            materialDialogMode = DialogMode::None;
            closeMaterialDialogRequested = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
    return result;
}
