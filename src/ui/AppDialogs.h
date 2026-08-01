#pragma once

#include <string>

#include "editor/EditorState.h"
#include "model/ProjectDocument.h"
#include "analysis/AnalysisState.h"
#include "ui/DialogResult.h"
#include "ui/ToolbarDialogRequest.h"

class AppDialogs
{
public:
    void ApplyRequest(ToolbarDialogRequest request, const ProjectDocument& document, const EditorState& state);
    DialogFrameResult Draw(const ProjectDocument& document, const EditorState& state, const AnalysisState& analysisState);
    bool HandleEscape();

private:
    enum class DialogMode
    {
        None,
        Create,
        Edit
    };

private:
    void BeginCreateMaterial(const ProjectDocument& document);
    void BeginEditMaterial(const ProjectDocument& document, const EditorState& state);
    MaterialDialogResult DrawMaterialDialog(const ProjectDocument& document, const EditorState& state);

    void BeginCreateSection(const ProjectDocument& document);
    void BeginEditSection(const ProjectDocument& document, const EditorState& state);
    SectionDialogResult DrawSectionDialog(const ProjectDocument& document, const EditorState& state);
    BeamDistanceDialogResult DrawBeamDistanceInputDialog(const EditorState& state);

    AnalysisDialogResult DrawProgressDialog(const AnalysisState& analysisState) const;

    void CloseMaterialDialog();
    void CloseSectionDialog();

    float ComputeMaxTextWidth(std::initializer_list<const char*> labels) const;
    float ComputeDialogWidth(float labelWidth, float inputWidth, float unitWidth) const;
    bool BeginPropertyTable(const char* id, float labelWidth, float inputWidth, float unitWidth) const;
    void DrawDoubleInputRow(const char* label, const char* inputId, double& value, const char* unitLabel, const char* format) const;
    void DrawPositiveDoubleInputRow(
        const char* label,
        const char* inputId,
        double& value,
        double& lastValidValue,
        const char* unitLabel,
        const char* format) const;
    void DrawDialogButtons() const;

private:
    static constexpr float dialogInputWidth = 180.0f;
    static constexpr float dialogWindowPaddingX = 7.0f;
    static constexpr float dialogWindowPaddingY = 7.0f;
    static constexpr float dialogButtonWidth = 110.0f;

    DialogMode materialDialogMode = DialogMode::None;
    DialogMode sectionDialogMode = DialogMode::None;

    bool closeMaterialDialogRequested = false;
    bool closeSectionDialogRequested = false;

    std::string materialName;
    double materialYoungModulus = 0.0;
    double materialThermalExpansion = 0.0;
    double materialYoungModulusLastValid = 0.0;
    double materialThermalExpansionLastValid = 0.0;
    
    std::string sectionName;
    double sectionArea = 0.0;
    double sectionInertia = 0.0;
    double sectionAreaLastValid = 0.0;
    double sectionInertiaLastValid = 0.0;
    std::string beamDistanceInput;
    bool beamDistanceInputShouldFocus = false;
    bool beamDistanceDialogWasOpen = false;

    mutable bool analysisPopupWasVisible = false;
    mutable float analysisPopupDisplayProgress = 0.0f;

};
