#pragma once

#include "editor/FrameRequests.h"
#include "editor/EditorState.h"
#include "model/ProjectDocument.h"

#include "analysis/AnalysisState.h"

class TopToolbar
{
public:
    void FillRequests(
        FrameRequests& requests,
        ProjectDocument& projectDocument,
        EditorState& editorState,
        const AnalysisState& analysisState,
        float cameraZoom);

private:
    void DrawHomeTab(
        FrameRequests& requests,
        ProjectDocument& projectDocument,
        EditorState& editorState,
        float cameraZoom);
    void DrawLoadsTab(FrameRequests& requests, ProjectDocument& projectDocument, EditorState& editorState);
    void DrawPropertiesTab(FrameRequests& requests, ProjectDocument& projectDocument, EditorState& editorState);
    void DrawDiscretizationTab(FrameRequests& requests, ProjectDocument& projectDocument, EditorState& editorState);
    void DrawImperfectionsTab(
        FrameRequests& requests,
        ProjectDocument& projectDocument,
        EditorState& editorState,
        const AnalysisState& analysisState);
    void DrawAnalysisTab(
        FrameRequests& requests,
        ProjectDocument& projectDocument,
        EditorState& editorState,
        const AnalysisState& analysisState);
    void DrawResultsTab(
        FrameRequests& requests,
        ProjectDocument& projectDocument,
        EditorState& editorState,
        const AnalysisState& analysisState);
    void DrawAboutTab();
};
