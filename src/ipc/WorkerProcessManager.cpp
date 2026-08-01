#include "ipc/WorkerProcessManager.h"
#include <iostream>
#include <filesystem>

#if !defined(PLATFORM_WEB)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include "engine/SimulationEngine.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#endif

namespace fs = std::filesystem;

std::string WorkerProcessManager::ResolveWorkerPath(const std::string& baseName)
{
#if !defined(PLATFORM_WEB)
    // 1. Verificar no diretório do executável atualmente em execução (ex: build/AppGUI.exe)
    char exePathBuf[MAX_PATH];
    if (GetModuleFileNameA(nullptr, exePathBuf, MAX_PATH) > 0)
    {
        fs::path currentExeDir = fs::path(exePathBuf).parent_path();
        fs::path pathInSameDir = currentExeDir / baseName;
        if (fs::exists(pathInSameDir))
        {
            return pathInSameDir.string();
        }
    }

    // 2. Verificar no diretório de trabalho atual (CWD) ou pasta build/
    if (fs::exists(baseName)) return baseName;

    fs::path buildSubdirPath = fs::path("build") / baseName;
    if (fs::exists(buildSubdirPath)) return buildSubdirPath.string();
#endif

    return baseName;
}

namespace
{
#if !defined(PLATFORM_WEB)
    bool WriteAllToHandle(HANDLE handle, const std::vector<std::uint8_t>& data)
    {
        DWORD totalWritten = 0;
        while (totalWritten < data.size())
        {
            DWORD written = 0;
            if (!WriteFile(handle, data.data() + totalWritten, static_cast<DWORD>(data.size() - totalWritten), &written, nullptr) || written == 0)
            {
                return false;
            }
            totalWritten += written;
        }
        return true;
    }
#endif
}

WorkerProcessManager::~WorkerProcessManager()
{
    Terminate();
}

void WorkerProcessManager::CleanupHandles()
{
#if !defined(PLATFORM_WEB)
    if (m_stdoutRead) { CloseHandle(static_cast<HANDLE>(m_stdoutRead)); m_stdoutRead = nullptr; }
    if (m_stdinWrite) { CloseHandle(static_cast<HANDLE>(m_stdinWrite)); m_stdinWrite = nullptr; }
    if (m_threadHandle) { CloseHandle(static_cast<HANDLE>(m_threadHandle)); m_threadHandle = nullptr; }
    if (m_processHandle) { CloseHandle(static_cast<HANDLE>(m_processHandle)); m_processHandle = nullptr; }
#endif
}

void WorkerProcessManager::Terminate()
{
    m_running = false;
#if !defined(PLATFORM_WEB)
    if (m_processHandle)
    {
        TerminateProcess(static_cast<HANDLE>(m_processHandle), 0);
    }
    CleanupHandles();
#endif
}

bool WorkerProcessManager::StartWorker(const std::string& workerPath, const std::string& jsonInput)
{
    Terminate();

    m_running = true;
    m_resultReady = false;
    m_progressUpdated = false;
    m_readBuffer.clear();

    const std::vector<std::uint8_t> payload = WorkerProtocol::StringToPayload(jsonInput);
    const std::vector<std::uint8_t> frame = WorkerProtocol::BuildFrame(WorkerProtocol::MessageType::JobRequest, payload);

#if !defined(PLATFORM_WEB)
    SECURITY_ATTRIBUTES saAttr{};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    HANDLE childStdoutRead = nullptr, childStdoutWrite = nullptr;
    HANDLE childStdinRead = nullptr, childStdinWrite = nullptr;

    if (!CreatePipe(&childStdoutRead, &childStdoutWrite, &saAttr, 0) ||
        !CreatePipe(&childStdinRead, &childStdinWrite, &saAttr, 0))
    {
        std::cerr << "[WorkerProcessManager] Error: CreatePipe failed." << std::endl;
        m_running = false;
        return false;
    }

    SetHandleInformation(childStdoutRead, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(childStdinWrite, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA siStartInfo{};
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = childStdoutWrite;
    siStartInfo.hStdOutput = childStdoutWrite;
    siStartInfo.hStdInput = childStdinRead;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    std::string commandLine = "\"" + workerPath + "\"";
    PROCESS_INFORMATION piProcInfo{};

    std::cout << "[WorkerProcessManager] Launching worker process: " << workerPath << std::endl;

    if (!CreateProcessA(
            workerPath.c_str(),
            commandLine.data(),
            nullptr, nullptr, TRUE,
            CREATE_NO_WINDOW,
            nullptr, nullptr,
            &siStartInfo, &piProcInfo))
    {
        std::cerr << "[WorkerProcessManager] Error: CreateProcessA failed for path: " << workerPath << " (Error code: " << GetLastError() << ")" << std::endl;
        CloseHandle(childStdoutRead);
        CloseHandle(childStdoutWrite);
        CloseHandle(childStdinRead);
        CloseHandle(childStdinWrite);
        m_running = false;
        return false;
    }

    CloseHandle(childStdoutWrite);
    CloseHandle(childStdinRead);

    m_processHandle = piProcInfo.hProcess;
    m_threadHandle = piProcInfo.hThread;
    m_stdoutRead = childStdoutRead;
    m_stdinWrite = childStdinWrite;

    if (!WriteAllToHandle(static_cast<HANDLE>(m_stdinWrite), frame))
    {
        std::cerr << "[WorkerProcessManager] Error: Failed to write frame to stdin." << std::endl;
        Terminate();
        return false;
    }

    CloseHandle(static_cast<HANDLE>(m_stdinWrite));
    m_stdinWrite = nullptr;
    return true;
#else
    rapidjson::Document doc;
    doc.Parse(jsonInput.c_str());
    if (!doc.HasParseError() && doc.HasMember("matrixSize") && doc.HasMember("loadFactor"))
    {
        SimulationEngine::SimulationInput input;
        input.matrixSize = doc["matrixSize"].GetInt();
        input.loadFactor = doc["loadFactor"].GetDouble();

        SimulationEngine engine;
        auto res = engine.RunSimulation(input, nullptr);

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(res.success);
        writer.Key("message"); writer.String(res.message.c_str());
        writer.Key("executionTimeMs"); writer.Double(res.executionTimeMs);
        writer.Key("solution");
        writer.StartArray();
        for (double v : res.solutionVector) writer.Double(v);
        writer.EndArray();
        writer.EndObject();

        m_latestResultJson = sb.GetString();
        m_resultReady = true;
        m_running = false;
        return true;
    }
    m_running = false;
    return false;
#endif
}

void WorkerProcessManager::Update()
{
    if (!m_running) return;

#if !defined(PLATFORM_WEB)
    DWORD availableBytes = 0;
    if (m_stdoutRead != nullptr &&
        PeekNamedPipe(static_cast<HANDLE>(m_stdoutRead), nullptr, 0, nullptr, &availableBytes, nullptr) &&
        availableBytes > 0)
    {
        std::vector<std::uint8_t> chunk(availableBytes);
        DWORD bytesRead = 0;
        if (ReadFile(static_cast<HANDLE>(m_stdoutRead), chunk.data(), availableBytes, &bytesRead, nullptr) && bytesRead > 0)
        {
            m_readBuffer.insert(m_readBuffer.end(), chunk.begin(), chunk.begin() + bytesRead);
        }
    }
#endif

    WorkerProtocol::MessageType msgType;
    std::vector<std::uint8_t> payload;

    while (WorkerProtocol::TryConsumeFrame(m_readBuffer, msgType, payload))
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (msgType == WorkerProtocol::MessageType::JobSucceeded)
        {
            m_latestResultJson = WorkerProtocol::PayloadToString(payload);
            m_resultReady = true;
            m_running = false;
            CleanupHandles();
            return;
        }
        else if (msgType == WorkerProtocol::MessageType::ProgressUpdate)
        {
            const std::string str = WorkerProtocol::PayloadToString(payload);
            m_latestProgress.status = str;
            m_progressUpdated = true;
        }
        else if (msgType == WorkerProtocol::MessageType::JobFailed)
        {
            m_running = false;
            CleanupHandles();
            return;
        }
    }
}

std::string WorkerProcessManager::GetLatestResult()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_resultReady = false;
    return m_latestResultJson;
}

WorkerProtocol::ProgressMessage WorkerProcessManager::GetLatestProgress()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_progressUpdated = false;
    return m_latestProgress;
}
