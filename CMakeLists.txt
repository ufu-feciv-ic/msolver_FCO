cmake_minimum_required(VERSION 3.16)
project(AppEngineTemplate CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RAYLIB_PATH "C:/raylib/raylib" CACHE PATH "Caminho da instalação do Raylib")

include_directories(
    src/
    worker/
    external/
    external/cereal/include
    external/imgui
    external/rlimgui
    external/eigen
    ${RAYLIB_PATH}/src
)

set(ENGINE_SOURCES
    worker/engine/AnalyticalIntegration.cpp
    worker/engine/ConcreteProperties.cpp
    worker/engine/InternalForces.cpp
    worker/engine/MomentSolver.cpp
    worker/engine/Point.cpp
    worker/engine/Polygon.cpp
    worker/engine/PolygonStressRegions.cpp
    worker/engine/Reinforcement.cpp
    worker/engine/Section.cpp
    worker/engine/SimulationEngine.cpp
    worker/engine/SteelProperties.cpp
    worker/engine/StrainDistribution.cpp
    worker/engine/combination.cpp
)

# 1. Executável do Motor (Worker Process)
add_executable(analysis-worker
    worker/main.cpp
    ${ENGINE_SOURCES}
    src/protocol/WorkerProtocol.cpp
)

# 2. Executável da GUI Frontend
add_executable(AppGUI
    src/main.cpp
    src/ui/AppUI.cpp
    src/ipc/WorkerProcessManager.cpp
    src/protocol/WorkerProtocol.cpp
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/implot.cpp
    external/imgui/implot_items.cpp
    external/rlimgui/rlImGui.cpp
)

if(WIN32)
    target_link_libraries(AppGUI PRIVATE ${RAYLIB_PATH}/src/libraylib.a opengl32 gdi32 winmm)
else()
    find_package(raylib REQUIRED)
    target_link_libraries(AppGUI PRIVATE raylib)
endif()

# 3. Suíte de Testes Integrada
enable_testing()

add_executable(run_tests
    tests/test_runner.cpp
    tests/test_protocol.cpp
    tests/test_engine.cpp
    ${ENGINE_SOURCES}
    src/protocol/WorkerProtocol.cpp
)

add_test(NAME TemplateTestSuite COMMAND run_tests)
