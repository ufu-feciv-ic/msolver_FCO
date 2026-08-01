# C++ App Engine Template (Raylib + ImGui + ImPlot + Eigen + IPC Worker)

Template genérico e reutilizável para projetos de engenharia, simulação e visualização em C++.

## 🏛 Arquitetura

Este template separa rigorosamente a **Interface de Usuário (Frontend)** do **Motor Computacional (Backend)** utilizando um processo Worker secundário e comunicação por IPC (Pipes stdin/stdout com *Binary Framing* + JSON).

```text
┌─────────────────────────────────────────┐          Pipes Stdin/Stdout           ┌─────────────────────────────────────┐
│             FRONTEND (AppGUI)           │ ───────────── IPC Frame ────────────► │        BACKEND (analysis-worker)    │
│  - Raylib (Janela / OpenGL Context)     │                                       │  - Headless Executable              │
│  - Dear ImGui (Controles / Inputs)      │ ◄────────── Progress & Result ─────── │  - Eigen (Matrizes & Solvers)       │
│  - ImPlot (Gráficos 2D em Tempo Real)   │                                       │  - RapidJSON (Serialização/DTO)     │
└─────────────────────────────────────────┘                                       └─────────────────────────────────────┘
```

### Vantagens
1. **Interface a 60+ FPS:** O motor numérico roda em um processo isolado, evitando qualquer congelamento (*stuttering*) na UI.
2. **Isolamento contra Crashes:** Se o motor falhar durante um cálculo complexo do Eigen, o executável GUI permanece aberto exibindo o erro.
3. **Multiplataforma:** Compatível com Windows (Win32 Anonymous Pipes) e WebAssembly (Emscripten Web Workers).

---

## 📁 Estrutura de Pastas

```text
├── external/             # Submódulos e dependências (Eigen, ImGui, ImPlot, RapidJSON, rlImGui)
├── src/                  # FRONTEND - Código da Interface
│   ├── main.cpp          # Entry point da janela (Raylib Init & Loop)
│   ├── ipc/              # Gerenciador de processos (WorkerProcessManager)
│   ├── protocol/         # Framing de dados (WorkerProtocol)
│   └── ui/               # Componentes ImGui & Gráficos ImPlot
├── worker/               # BACKEND - Código do Motor Computacional
│   ├── main.cpp          # Entry point do executável Headless Worker
│   └── engine/           # Algoritmos numéricos (SimulationEngine com Eigen)
├── Makefile              # Build via mingw32-make / w64devkit
└── CMakeLists.txt        # Build via CMake / VS Code / CLion
```

---

## 🛠 Como Compilar e Executar

### Utilizando Makefile (w64devkit / MinGW):
```bash
make
make run
```

Os executáveis serão gerados na pasta `build/`:
- `build/AppGUI.exe`
- `build/analysis-worker.exe`

### Utilizando CMake:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

---

## 🚀 Como Usar Este Template para um Novo Projeto

1. Faça checkout ou clone desta branch `template`.
2. Para adicionar novas variáveis de entrada/saída:
   - Altere o JSON em [src/ui/AppUI.cpp](file:///C:/Users/Arthur/Documents/GitHub/2d-Editor/src/ui/AppUI.cpp)
   - Adicione seus cálculos no Eigen em [worker/engine/SimulationEngine.cpp](file:///C:/Users/Arthur/Documents/GitHub/2d-Editor/worker/engine/SimulationEngine.cpp)
3. Para adicionar novos gráficos:
   - Adicione chamadas do `ImPlot::PlotLine` ou `ImPlot::PlotScatter` no método `RenderPlotPanel()` do `AppUI`.
