// Temporary end-to-end driver: publishes geometry commands to a running EnginePlatform
// and prints the result packets. Removed once the Gangti.stp import is verified.
#include <Windows.h>

#include <charconv>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>

#include "Command/CommandIds.hpp"
#include "Command/CommandQueueName.hpp"
#include "Common/Protocol.hpp"
#include "Common/Subscriber.hpp"

namespace
{
    struct ImportGeometryParamPOD
    {
        char filePath[512];
        char kernelName[64];
    };

    struct GeometryFeaturesParamPOD
    {
        uint64_t dataNodeId;
        char name[64];
        uint32_t featureType;
        uint32_t offset;
        uint32_t limit;
        uint32_t reserved;
    };

    struct GeometryImportResultPOD
    {
        uint32_t magic;
        uint32_t version;
        uint32_t status;
        uint32_t flags;
        uint64_t dataNodeId;
        uint32_t vertexCount;
        uint32_t edgeCount;
        uint32_t faceCount;
        uint32_t solidCount;
        uint32_t triangleCount;
        uint32_t reserved;
        float bboxMin[3];
        float bboxMax[3];
    };

    bool pushPacket(uint32_t processId, uint32_t eventId, const void *payload, uint32_t payloadSize)
    {
        EgLab::Common::DynamicArray<uint8_t> packet;
        packet.resize(sizeof(uint32_t) + payloadSize);
        std::memcpy(packet.data(), &payloadSize, sizeof(payloadSize));
        std::memcpy(packet.data() + sizeof(uint32_t), payload, payloadSize);
        EgLab::Common::MemoryQueue queue(EgLab::Platform::commandMemoryName(processId), false);
        return queue.push(eventId, packet.data(), static_cast<uint32_t>(packet.size()));
    }

    bool waitResult(uint32_t processId, uint32_t eventId, EgLab::Common::DynamicArray<uint8_t> &out,
                    int timeoutMs)
    {
        EgLab::Common::MemoryQueue queue(EgLab::Platform::meshDataResultMemoryName(processId),
                                         false);
        EgLab::Common::Subscriber subscriber(queue);
        subscriber.subscrib(eventId);
        for (int elapsed = 0; elapsed < timeoutMs; elapsed += 100)
        {
            uint32_t receivedId = 0;
            if (subscriber.tryReceiveDynamic(receivedId, out))
            {
                if (out.size() < sizeof(uint32_t)) continue;
                uint32_t dataSize = 0;
                std::memcpy(&dataSize, out.data(), sizeof(dataSize));
                if (dataSize <= out.size() - sizeof(uint32_t)) return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return false;
    }
} // namespace

int main(int argc, char **argv)
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 3)
    {
        std::printf("usage: geometry_import_sender <pid> <file.stp>\n");
        return 2;
    }
    uint32_t processId = 0;
    const std::string pidArgument(argv[1]);
    const auto parse = std::from_chars(pidArgument.data(), pidArgument.data() + pidArgument.size(),
                                       processId);
    if (parse.ec != std::errc{} || processId == 0)
    {
        std::printf("invalid pid\n");
        return 2;
    }

    ImportGeometryParamPOD import{};
    std::strncpy(import.filePath, argv[2], sizeof(import.filePath) - 1);
    if (!pushPacket(processId, static_cast<uint32_t>(EgLab::Platform::EventId::GeometryImport),
                    &import, sizeof(import)))
    {
        std::printf("push GeometryImport failed\n");
        return 1;
    }
    std::printf("GeometryImport pushed (%zu bytes)\n", sizeof(import));

    EgLab::Common::DynamicArray<uint8_t> packet;
    if (!waitResult(processId,
                    static_cast<uint32_t>(EgLab::Platform::EventId::GeometryImport), packet, 120000))
    {
        std::printf("no import result\n");
        return 1;
    }
    GeometryImportResultPOD result{};
    std::memcpy(&result, packet.data() + sizeof(uint32_t), sizeof(result));
    std::printf("import status=%u dataNodeId=%llu V=%u E=%u F=%u S=%u tris=%u bbox=[%f %f %f]-[%f "
                "%f %f]\n",
                result.status, static_cast<unsigned long long>(result.dataNodeId),
                result.vertexCount, result.edgeCount, result.faceCount, result.solidCount,
                result.triangleCount, result.bboxMin[0], result.bboxMin[1], result.bboxMin[2],
                result.bboxMax[0], result.bboxMax[1], result.bboxMax[2]);

    GeometryFeaturesParamPOD features{};
    features.dataNodeId = result.dataNodeId;
    features.featureType = 3; // all
    features.limit = 32;
    if (!pushPacket(processId,
                    static_cast<uint32_t>(EgLab::Platform::EventId::ListGeometryFeatures), &features,
                    sizeof(features)))
    {
        std::printf("push ListGeometryFeatures failed\n");
        return 1;
    }
    if (!waitResult(processId,
                    static_cast<uint32_t>(EgLab::Platform::EventId::ListGeometryFeatures), packet,
                    30000))
    {
        std::printf("no feature list result\n");
        return 1;
    }
    uint32_t header[8]{};
    std::memcpy(header, packet.data() + sizeof(uint32_t), sizeof(header));
    std::printf("features magic=0x%08X status=%u total=%u returned=%u\n", header[0], header[2],
                header[3], header[5]);
    return 0;
}
