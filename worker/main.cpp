#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#endif

#include "engine/SimulationEngine.h"
#include "protocol/WorkerProtocol.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

void SetBinaryMode()
{
#if defined(_WIN32)
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif
}

void SendFrame(WorkerProtocol::MessageType type, const std::string& jsonContent)
{
    const auto payload = WorkerProtocol::StringToPayload(jsonContent);
    const auto frame = WorkerProtocol::BuildFrame(type, payload);
    std::cout.write(reinterpret_cast<const char*>(frame.data()), frame.size());
    std::cout.flush();
}

int main()
{
    SetBinaryMode();

    std::vector<std::uint8_t> buffer;
    char chunk[4096];

    while (std::cin.read(chunk, sizeof(chunk)) || std::cin.gcount() > 0)
    {
        const std::streamsize bytesRead = std::cin.gcount();
        buffer.insert(buffer.end(), chunk, chunk + bytesRead);

        WorkerProtocol::MessageType type;
        std::vector<std::uint8_t> payload;

        if (WorkerProtocol::TryConsumeFrame(buffer, type, payload))
        {
            if (type == WorkerProtocol::MessageType::JobRequest)
            {
                const std::string jsonInput = WorkerProtocol::PayloadToString(payload);
                
                rapidjson::Document doc;
                doc.Parse(jsonInput.c_str());

                SimulationEngine::SimulationInput input;
                if (!doc.HasParseError())
                {
                    if (doc.HasMember("matrixSize") && doc["matrixSize"].IsInt())
                        input.matrixSize = doc["matrixSize"].GetInt();
                    if (doc.HasMember("loadFactor") && doc["loadFactor"].IsDouble())
                        input.loadFactor = doc["loadFactor"].GetDouble();
                }

                SimulationEngine engine;
                SimulationEngine::SimulationResult result = engine.RunSimulation(
                    input,
                    [](float progress, const std::string& status)
                    {
                        rapidjson::StringBuffer sb;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
                        writer.StartObject();
                        writer.Key("progress");
                        writer.Double(progress);
                        writer.Key("status");
                        writer.String(status.c_str());
                        writer.EndObject();

                        SendFrame(WorkerProtocol::MessageType::ProgressUpdate, sb.GetString());
                    }
                );

                // Serializar resultado final
                rapidjson::StringBuffer sb;
                rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
                writer.StartObject();
                writer.Key("success");
                writer.Bool(result.success);
                writer.Key("executionTimeMs");
                writer.Double(result.executionTimeMs);
                writer.Key("message");
                writer.String(result.message.c_str());
                writer.Key("solution");
                writer.StartArray();
                for (double val : result.solutionVector)
                {
                    writer.Double(val);
                }
                writer.EndArray();
                writer.EndObject();

                SendFrame(WorkerProtocol::MessageType::JobSucceeded, sb.GetString());
                return 0;
            }
        }
    }

    return 0;
}
