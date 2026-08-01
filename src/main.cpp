#include "raylib.h"
#include "imgui.h"
#include "implot.h"
#include "rlImGui.h"
#include "ui/AppUI.h"

int main()
{
    // 1. Inicializar Janela Raylib
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Template C++: Raylib + ImGui + ImPlot + Eigen + IPC Worker");
    SetTargetFPS(60);

    // 2. Setup do rlImGui e ImPlot
    rlImGuiSetup(true);
    ImPlot::CreateContext();

    // 3. Instância da Aplicação UI
    AppUI appUI;

    // Loop Principal
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));

        rlImGuiBegin();

        appUI.UpdateAndRender();

        rlImGuiEnd();
        EndDrawing();
    }

    // Teardown
    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}