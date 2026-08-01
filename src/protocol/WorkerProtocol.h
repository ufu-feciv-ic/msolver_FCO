#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace WorkerProtocol
{
    constexpr std::uint32_t FrameMagic = 0x454E474Eu; // "ENGN"
    constexpr std::uint32_t ProtocolVersion = 1u;

    enum class MessageType : std::uint32_t
    {
        JobRequest = 1,
        ProgressUpdate = 2,
        JobSucceeded = 3,
        JobFailed = 4
    };

    struct FrameHeader
    {
        std::uint32_t magic = FrameMagic;
        std::uint32_t version = ProtocolVersion;
        std::uint32_t type = 0;
        std::uint32_t payloadSize = 0;
    };

    struct ProgressMessage
    {
        std::string status;
        float progress = 0.0f; // 0.0 to 1.0
    };

    // Frame creation & parsing
    std::vector<std::uint8_t> BuildFrame(MessageType type, const std::vector<std::uint8_t>& payload);
    bool TryConsumeFrame(std::vector<std::uint8_t>& buffer, MessageType& type, std::vector<std::uint8_t>& payload);

    // Helpers to build JSON payloads
    std::vector<std::uint8_t> StringToPayload(const std::string& str);
    std::string PayloadToString(const std::vector<std::uint8_t>& payload);
}
