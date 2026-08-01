#include "protocol/WorkerProtocol.h"
#include <cstring>

namespace WorkerProtocol
{
    std::vector<std::uint8_t> BuildFrame(MessageType type, const std::vector<std::uint8_t>& payload)
    {
        FrameHeader header;
        header.magic = FrameMagic;
        header.version = ProtocolVersion;
        header.type = static_cast<std::uint32_t>(type);
        header.payloadSize = static_cast<std::uint32_t>(payload.size());

        std::vector<std::uint8_t> frame(sizeof(FrameHeader) + payload.size());
        std::memcpy(frame.data(), &header, sizeof(FrameHeader));
        if (!payload.empty())
        {
            std::memcpy(frame.data() + sizeof(FrameHeader), payload.data(), payload.size());
        }
        return frame;
    }

    bool TryConsumeFrame(std::vector<std::uint8_t>& buffer, MessageType& type, std::vector<std::uint8_t>& payload)
    {
        if (buffer.size() < sizeof(FrameHeader))
        {
            return false;
        }

        FrameHeader header;
        std::memcpy(&header, buffer.data(), sizeof(FrameHeader));

        if (header.magic != FrameMagic || header.version != ProtocolVersion)
        {
            // Corrupted buffer; discard first byte to resynchronize
            buffer.erase(buffer.begin());
            return false;
        }

        const std::size_t totalSize = sizeof(FrameHeader) + header.payloadSize;
        if (buffer.size() < totalSize)
        {
            return false; // Waiting for complete frame
        }

        type = static_cast<MessageType>(header.type);
        payload.assign(buffer.begin() + sizeof(FrameHeader), buffer.begin() + totalSize);
        buffer.erase(buffer.begin(), buffer.begin() + totalSize);
        return true;
    }

    std::vector<std::uint8_t> StringToPayload(const std::string& str)
    {
        return std::vector<std::uint8_t>(str.begin(), str.end());
    }

    std::string PayloadToString(const std::vector<std::uint8_t>& payload)
    {
        return std::string(payload.begin(), payload.end());
    }
}
