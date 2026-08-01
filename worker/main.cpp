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
                    if (doc.HasMember("polygon") && doc["polygon"].IsArray())
                    {
                        for (const auto& v : doc["polygon"].GetArray())
                        {
                            if (v.IsObject() && v.HasMember("x") && v.HasMember("y"))
                            {
                                input.polygonVertices.push_back(Point(v["x"].GetDouble(), v["y"].GetDouble()));
                            }
                        }
                    }
                    if (doc.HasMember("rebars") && doc["rebars"].IsArray())
                    {
                        for (const auto& r : doc["rebars"].GetArray())
                        {
                            if (r.IsObject() && r.HasMember("x") && r.HasMember("y") && r.HasMember("diameter"))
                            {
                                SimulationEngine::RebarBarInput bar;
                                bar.x = r["x"].GetDouble();
                                bar.y = r["y"].GetDouble();
                                bar.diameter = r["diameter"].GetDouble();
                                input.rebars.push_back(bar);
                            }
                        }
                    }
                    if (doc.HasMember("fck") && doc["fck"].IsDouble()) input.fck = doc["fck"].GetDouble();
                    if (doc.HasMember("gammaC") && doc["gammaC"].IsDouble()) input.gammaC = doc["gammaC"].GetDouble();
                    if (doc.HasMember("concreteModel") && doc["concreteModel"].IsInt()) input.concreteModelType = doc["concreteModel"].GetInt();

                    if (doc.HasMember("fyk") && doc["fyk"].IsDouble()) input.fyk = doc["fyk"].GetDouble();
                    if (doc.HasMember("gammaS") && doc["gammaS"].IsDouble()) input.gammaS = doc["gammaS"].GetDouble();
                    if (doc.HasMember("Es") && doc["Es"].IsDouble()) input.Es = doc["Es"].GetDouble();

                    if (doc.HasMember("Nsd") && doc["Nsd"].IsDouble()) input.Nsd = doc["Nsd"].GetDouble();
                    if (doc.HasMember("Msdx") && doc["Msdx"].IsDouble()) input.Msdx = doc["Msdx"].GetDouble();
                    if (doc.HasMember("Msdy") && doc["Msdy"].IsDouble()) input.Msdy = doc["Msdy"].GetDouble();
                }

                bool isSizingMode = (doc.HasMember("mode") && doc["mode"].IsString() && std::string(doc["mode"].GetString()) == "sizing");

                SimulationEngine engine;
                auto progressCallback = [](float progress, const std::string& status)
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
                };

                SimulationEngine::SimulationResult result;
                SimulationEngine::SizingResult sizingResult;

                if (isSizingMode)
                {
                    sizingResult = engine.RunSizingSimulation(input, progressCallback);
                    result = sizingResult.verificationResult;
                    result.message = sizingResult.message;
                    result.executionTimeMs = sizingResult.executionTimeMs;
                }
                else
                {
                    result = engine.RunSimulation(input, progressCallback);
                }

                // Serializar resultado final
                rapidjson::StringBuffer sb;
                rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
                writer.StartObject();
                writer.Key("success");
                writer.Bool(isSizingMode ? sizingResult.success : result.success);
                writer.Key("executionTimeMs");
                writer.Double(result.executionTimeMs);
                writer.Key("message");
                writer.String(result.message.c_str());

                writer.Key("isSafe");
                writer.Bool(result.isSafe);
                writer.Key("area");
                writer.Double(result.area);
                writer.Key("height");
                writer.Double(result.height);
                writer.Key("centroidX");
                writer.Double(result.centroid.getX());
                writer.Key("centroidY");
                writer.Double(result.centroid.getY());
                writer.Key("inertiaX");
                writer.Double(result.inertiaX);
                writer.Key("inertiaY");
                writer.Double(result.inertiaY);

                writer.Key("fcd");
                writer.Double(result.fcd);
                writer.Key("fyd");
                writer.Double(result.fyd);

                writer.Key("Nsd");
                writer.Double(result.Nsd);
                writer.Key("Msdx");
                writer.Double(result.Msdx);
                writer.Key("Msdy");
                writer.Double(result.Msdy);

                if (isSizingMode)
                {
                    writer.Key("isSizing");
                    writer.Bool(true);
                    writer.Key("requiredSteelArea");
                    writer.Double(sizingResult.requiredSteelArea);
                    writer.Key("steelRatioPercent");
                    writer.Double(sizingResult.steelRatioPercent);
                    writer.Key("omegaMechanicalRatio");
                    writer.Double(sizingResult.omegaMechanicalRatio);
                    writer.Key("nuReducedNormal");
                    writer.Double(sizingResult.nuReducedNormal);
                    writer.Key("muXReducedMoment");
                    writer.Double(sizingResult.muXReducedMoment);
                    writer.Key("muYReducedMoment");
                    writer.Double(sizingResult.muYReducedMoment);
                    writer.Key("iterationsCount");
                    writer.Int(sizingResult.iterationsCount);
                }

                writer.Key("envelopeMoments");
                writer.StartArray();
                for (const auto& pt : result.envelopeMoments)
                {
                    writer.StartObject();
                    writer.Key("x");
                    writer.Double(pt.getX());
                    writer.Key("y");
                    writer.Double(pt.getY());
                    writer.EndObject();
                }
                writer.EndArray();

                writer.Key("polygon");
                writer.StartArray();
                for (const auto& pt : result.polygonVertices)
                {
                    writer.StartObject();
                    writer.Key("x");
                    writer.Double(pt.getX());
                    writer.Key("y");
                    writer.Double(pt.getY());
                    writer.EndObject();
                }
                writer.EndArray();

                writer.Key("rebars");
                writer.StartArray();
                const auto& rebarList = isSizingMode ? sizingResult.sizedRebars : result.rebars;
                for (const auto& r : rebarList)
                {
                    writer.StartObject();
                    writer.Key("x");
                    writer.Double(r.x);
                    writer.Key("y");
                    writer.Double(r.y);
                    writer.Key("diameter");
                    writer.Double(r.diameter);
                    writer.EndObject();
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
