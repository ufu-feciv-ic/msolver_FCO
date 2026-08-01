#include <iostream>
#include <cassert>
#include "protocol/WorkerProtocol.h"

void TestProtocolFraming()
{
    std::cout << "[TEST] Running TestProtocolFraming..." << std::endl;

    std::string testJson = "{\"matrixSize\":20,\"loadFactor\":2.5}";
    auto payload = WorkerProtocol::StringToPayload(testJson);
    
    // Build frame
    auto frame = WorkerProtocol::BuildFrame(WorkerProtocol::MessageType::JobRequest, payload);
    assert(!frame.empty());

    // Consume frame
    WorkerProtocol::MessageType consumedType;
    std::vector<std::uint8_t> consumedPayload;
    bool success = WorkerProtocol::TryConsumeFrame(frame, consumedType, consumedPayload);

    assert(success == true);
    assert(consumedType == WorkerProtocol::MessageType::JobRequest);

    std::string reconstructedJson = WorkerProtocol::PayloadToString(consumedPayload);
    assert(reconstructedJson == testJson);

    std::cout << "[TEST PASSED] TestProtocolFraming" << std::endl;
}

void TestProtocolCorruptedFrame()
{
    std::cout << "[TEST] Running TestProtocolCorruptedFrame..." << std::endl;

    std::vector<std::uint8_t> corruptedBuffer = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };
    WorkerProtocol::MessageType consumedType;
    std::vector<std::uint8_t> consumedPayload;

    bool success = WorkerProtocol::TryConsumeFrame(corruptedBuffer, consumedType, consumedPayload);
    assert(success == false); // Should reject invalid magic/header

    std::cout << "[TEST PASSED] TestProtocolCorruptedFrame" << std::endl;
}
