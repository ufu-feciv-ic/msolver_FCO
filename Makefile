# =========================================================
# Makefile para Projeto Template (Raylib + ImGui + ImPlot + Eigen + RapidJSON)
# =========================================================

SHELL := cmd.exe
.SHELLFLAGS := /C

.DEFAULT_GOAL := all

PROJECT_NAME ?= AppGUI
WORKER_NAME  ?= analysis-worker
TEST_NAME    ?= run_tests

SRC_DIR      := src
WORKER_DIR   := worker
TESTS_DIR    := tests
EXTERNAL_DIR := external
OBJ_DIR      := obj
OBJ_WORKER_DIR := obj_worker
OBJ_TEST_DIR := obj_tests
BUILD_DIR    := build
BUILD_DIR_WEB:= $(BUILD_DIR)/web

RAYLIB_PATH   ?= C:/raylib/raylib
COMPILER_PATH ?= C:/raylib/w64devkit/bin

CXX  := $(COMPILER_PATH)/g++.exe
EMCC := em++

TARGET        := $(BUILD_DIR)/$(PROJECT_NAME).exe
WORKER_TARGET := $(BUILD_DIR)/$(WORKER_NAME).exe
TEST_TARGET   := $(BUILD_DIR)/$(TEST_NAME).exe
TARGET_WEB    := $(BUILD_DIR_WEB)/index.html

# ---------------------------------------------------------
# Build Mode (DEBUG / RELEASE)
# ---------------------------------------------------------
BUILD_MODE ?= RELEASE

STD_FLAGS     := -std=c++17
WARNING_FLAGS := -Wall -Wextra -Wno-missing-braces -Wno-missing-field-initializers

ifeq ($(BUILD_MODE),DEBUG)
OPT_FLAGS := -O0 -g -D_DEBUG
else
OPT_FLAGS := -O2
endif

CXXFLAGS := $(STD_FLAGS) $(OPT_FLAGS) $(WARNING_FLAGS) -Wa,-mbig-obj

INCLUDE_PATHS := -I.
INCLUDE_PATHS += -I$(SRC_DIR)
INCLUDE_PATHS += -I$(WORKER_DIR)
INCLUDE_PATHS += -I$(EXTERNAL_DIR)
INCLUDE_PATHS += -I$(EXTERNAL_DIR)/imgui
INCLUDE_PATHS += -I$(EXTERNAL_DIR)/rlimgui
INCLUDE_PATHS += -I$(EXTERNAL_DIR)/eigen
INCLUDE_PATHS += -I$(RAYLIB_PATH)/src

DEFINES := -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33

LDFLAGS := -L$(RAYLIB_PATH)/src
LDLIBS  := -lraylib -lopengl32 -lgdi32 -lwinmm

GUI_SRCS := $(SRC_DIR)/main.cpp \
            $(SRC_DIR)/ui/AppUI.cpp \
            $(SRC_DIR)/ipc/WorkerProcessManager.cpp \
            $(SRC_DIR)/protocol/WorkerProtocol.cpp \
            $(EXTERNAL_DIR)/imgui/imgui.cpp \
            $(EXTERNAL_DIR)/imgui/imgui_draw.cpp \
            $(EXTERNAL_DIR)/imgui/imgui_tables.cpp \
            $(EXTERNAL_DIR)/imgui/imgui_widgets.cpp \
            $(EXTERNAL_DIR)/imgui/implot.cpp \
            $(EXTERNAL_DIR)/imgui/implot_items.cpp \
            $(EXTERNAL_DIR)/rlimgui/rlImGui.cpp

WORKER_SRCS := $(WORKER_DIR)/main.cpp \
               $(WORKER_DIR)/engine/SimulationEngine.cpp \
               $(SRC_DIR)/protocol/WorkerProtocol.cpp

TEST_SRCS := $(TESTS_DIR)/test_runner.cpp \
             $(TESTS_DIR)/test_protocol.cpp \
             $(TESTS_DIR)/test_engine.cpp \
             $(WORKER_DIR)/engine/SimulationEngine.cpp \
             $(SRC_DIR)/protocol/WorkerProtocol.cpp

GUI_OBJS    := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(GUI_SRCS))
WORKER_OBJS := $(patsubst %.cpp,$(OBJ_WORKER_DIR)/%.o,$(WORKER_SRCS))
TEST_OBJS   := $(patsubst %.cpp,$(OBJ_TEST_DIR)/%.o,$(TEST_SRCS))

all: $(TARGET) $(WORKER_TARGET) $(TEST_TARGET)

$(TARGET): $(GUI_OBJS)
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	$(CXX) $(CXXFLAGS) $(GUI_OBJS) -o $@ $(LDFLAGS) $(LDLIBS)
	@echo [GUI Build Success - $(BUILD_MODE)] $@

$(WORKER_TARGET): $(WORKER_OBJS)
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	$(CXX) $(CXXFLAGS) $(WORKER_OBJS) -o $@
	@echo [Worker Build Success - $(BUILD_MODE)] $@

$(TEST_TARGET): $(TEST_OBJS)
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) -o $@
	@echo [Test Suite Build Success - $(BUILD_MODE)] $@

$(OBJ_DIR)/%.o: %.cpp
	@if not exist "$(dir $@)" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) $(DEFINES) -c $< -o $@

$(OBJ_WORKER_DIR)/%.o: %.cpp
	@if not exist "$(dir $@)" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) $(DEFINES) -c $< -o $@

$(OBJ_TEST_DIR)/%.o: %.cpp
	@if not exist "$(dir $@)" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) $(DEFINES) -c $< -o $@

# Web assembly target
FLAGS_WEB := -Os -Wall -std=c++17 -DPLATFORM_WEB -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1
web:
	@if not exist "$(BUILD_DIR_WEB)" mkdir "$(subst /,\,$(BUILD_DIR_WEB))"
	$(EMCC) $(GUI_SRCS) $(INCLUDE_PATHS) $(FLAGS_WEB) -o $(TARGET_WEB)
	@echo [WebAssembly Build Success] $(TARGET_WEB)

serve:
	python -m http.server --directory build/web 8000

clean:
	@if exist "$(OBJ_DIR)" rmdir /s /q "$(OBJ_DIR)"
	@if exist "$(OBJ_WORKER_DIR)" rmdir /s /q "$(OBJ_WORKER_DIR)"
	@if exist "$(OBJ_TEST_DIR)" rmdir /s /q "$(OBJ_TEST_DIR)"
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@echo Cleaned!

test: $(TEST_TARGET)
	@$(subst /,\,$(TEST_TARGET))

run: all
	$(TARGET)
