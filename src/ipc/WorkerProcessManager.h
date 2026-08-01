#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <vector>
#include <cstdint>
#include "protocol/WorkerProtocol.h"

class WorkerProcessManager
{
public:
    WorkerProcessManager() = default;
    ~WorkerProcessManager();

    static std::string ResolveWorkerPath(const std::string& baseName = "analysis-worker.exe");

    bool StartWorker(const std::string& workerPath, const std::string& jsonInput);
    void Update();
    void Terminate();

    bool IsRunning() const { return m_running; }
    bool HasNewResult() const { return m_resultReady; }
    bool HasProgressUpdate() const { return m_progressUpdated; }

    std::string GetLatestResult();
    WorkerProtocol::ProgressMessage GetLatestProgress();

private:
    void CleanupHandles();

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_resultReady{false};
    std::atomic<bool> m_progressUpdated{false};

    std::string m_latestResultJson;
    WorkerProtocol::ProgressMessage m_latestProgress;

    mutable std::mutex m_mutex;
    std::vector<std::uint8_t> m_readBuffer;

#if !defined(PLATFORM_WEB)
    void* m_processHandle = nullptr;
    void* m_threadHandle = nullptr;
    void* m_stdoutRead = nullptr;
    void* m_stdinWrite = nullptr;
#endif
};
