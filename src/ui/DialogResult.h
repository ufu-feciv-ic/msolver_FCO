#pragma once

#include <string>

#include "model/Point2D.h"

struct MaterialDialogResult
{
    enum class Action
    {
        None,
        Create,
        Update
    };

    Action action = Action::None;
    int targetMaterialId = -1;
    std::string name;
    double youngModulus = 0.0;
    double thermalExpansion = 0.0;
};

struct SectionDialogResult
{
    enum class Action
    {
        None,
        Create,
        Update
    };

    Action action = Action::None;
    int targetSectionId = -1;
    std::string name;
    double area = 0.0;
    double inertia = 0.0;
};

struct AppDialogResults
{
    MaterialDialogResult material;
    SectionDialogResult section;
};

struct BeamDistanceDialogResult
{
    enum class Action
    {
        None,
        Confirm,
        Cancel
    };

    Action action = Action::None;
    bool shouldCloseDialog = false;
    bool hasPreviewNode = false;
    Point2D previewNodeWorld{0.0, 0.0};
    Point2D createdNodeWorld{0.0, 0.0};
};

struct AnalysisDialogResult
{
    enum class Action
    {
        None,
        Abort,
        Close
    };

    Action action = Action::None;
};

struct DialogFrameResult
{
    AppDialogResults document;
    BeamDistanceDialogResult beamDistance;
    AnalysisDialogResult analysis;
};
