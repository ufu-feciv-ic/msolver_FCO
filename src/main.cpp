#include "raylib.h"
#include "imgui.h"
#include "implot.h"
#include "rlImGui.h"
#include "ui/AppUI.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static AppUI* g_appUI = nullptr;

static void UpdateDrawFrame()
{
    BeginDrawing();
    ClearBackground(GetColor(0x181818FF));

    rlImGuiBegin();

    if (g_appUI)
    {
        g_appUI->UpdateAndRender();
    }

    rlImGuiEnd();
    EndDrawing();
}

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
    g_appUI = &appUI;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    // Loop Principal
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }

    // Teardown
    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();
#endif

    return 0;
}