#include <Windows.h>

#include <charconv>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <vector>

#include "Command/CommandIds.hpp"
#include "Command/CommandQueueName.hpp"
#include "Command/MeshPODWire.hpp"
#include "Common/Protocol.hpp"
#include "MeshEngine/MeshData/Element.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/MeshPOD.hpp"
#include "MeshEngine/MeshData/MeshToPOD.hpp"
#include "MeshEngine/MeshData/Node.hpp"

namespace
{
    constexpr int maxNodeOfElem = 27;

    enum class SimpleElemType : int
    {
        ES_None = 0x0000,
        Node1_Hex = 0x3101,
        Node8_Hex = 0x3108,
        Node20_Hex = 0x3114,
        Node27_Hex = 0x311b,
        Node1_Tet = 0x3201,
        Node4_Tet = 0x3204,
        Node5_Tet = 0x3205,
        Node10_Tet = 0x320a,
        Node15_Tet = 0x320f,
        Node6_Wed = 0x3306,
        Node15_Wed = 0x330f,
        Node5_Prd = 0x3405,
        Node13_Prd = 0x340d,
        Node20_Hex_Edge12 = 0x3514,
        Node10_Tet_Edge6 = 0x360a,
        Node15_Wed_Edge9 = 0x370f,
        Node13_Prd_Edge9 = 0x380d,
        Node4_Quad = 0x2904,
        Node8_Quad = 0x2908,
        Node9_Quad = 0x2909,
        Node4_QuadBL = 0x2e04,
        Node8_QuadBL = 0x2e08,
        Node3_Tri = 0x2a03,
        Node6_Tri = 0x2a06,
        Node2_Line = 0x1b02,
        Node3_Line = 0x1b03,
        Node1_Point = 0x0c01,
        NodeX_Polyhedron = 0x3d00,
        ElemTypeNum = 0x001c
    };

    struct Node
    {
        int id{};
        double xyz[3]{};
        const std::string* geoId{};
        uint32_t sign{};
    };

    struct Elem
    {
        int id{};
        int type{};
        uint32_t sign{};
        int nodes[maxNodeOfElem]{};
    };

    struct SimpleMesh
    {
        std::vector<Node> nodes;
        std::vector<Elem> elems;
    };

    bool isEnginePlatformProcess(uint32_t processId)
    {
        HANDLE process =
            OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, processId);
        if (process == nullptr)
        {
            std::cerr << "Cannot open process " << processId << ": " << GetLastError() << '\n';
            return false;
        }

        char imagePath[32768]{};
        DWORD imagePathSize = static_cast<DWORD>(sizeof(imagePath));
        const bool gotImagePath =
            QueryFullProcessImageNameA(process, 0, imagePath, &imagePathSize) != FALSE;
        const bool isRunning = WaitForSingleObject(process, 0) == WAIT_TIMEOUT;
        CloseHandle(process);
        if (!gotImagePath || !isRunning)
        {
            std::cerr << "The specified process is not running or its image path is unavailable.\n";
            return false;
        }

        const char* executableName = std::strrchr(imagePath, '\\');
        executableName = executableName == nullptr ? imagePath : executableName + 1;
        if (_stricmp(executableName, "EnginePlatform.exe") != 0)
        {
            std::cerr << "PID " << processId << " is not EnginePlatform.exe: " << imagePath << '\n';
            return false;
        }
        return true;
    }

    SimpleMesh makeTestMesh()
    {
        SimpleMesh mesh;
        mesh.nodes = {
            {1, {0.0, 0.0, 0.0}, nullptr, 0}, {2, {1.0, 0.0, 0.0}, nullptr, 0},
            {3, {1.0, 1.0, 0.0}, nullptr, 0}, {4, {0.0, 1.0, 0.0}, nullptr, 0},
            {5, {0.0, 0.0, 1.0}, nullptr, 0}, {6, {1.0, 0.0, 1.0}, nullptr, 0},
            {7, {1.0, 1.0, 1.0}, nullptr, 0}, {8, {0.0, 1.0, 1.0}, nullptr, 0},
        };

        Elem elem{};
        elem.id = 1;
        elem.type = static_cast<int>(SimpleElemType::Node8_Hex);
        for (int& nodeId : elem.nodes)
        {
            nodeId = -1;
        }
        for (int index = 0; index < 8; ++index)
        {
            elem.nodes[index] = index + 1;
        }
        mesh.elems.push_back(elem);
        return mesh;
    }

    EgLab::ME::ElemType convertElemType(int type)
    {
        using EgLab::ME::ElemType;
        switch (static_cast<SimpleElemType>(type))
        {
        case SimpleElemType::Node8_Hex:
            return ElemType::Hex8;
        case SimpleElemType::Node20_Hex:
            return ElemType::Hex20;
        case SimpleElemType::Node4_Tet:
            return ElemType::Tet4;
        case SimpleElemType::Node10_Tet:
            return ElemType::Tet10;
        case SimpleElemType::Node6_Wed:
            return ElemType::Wed6;
        case SimpleElemType::Node5_Prd:
            return ElemType::Prd5;
        case SimpleElemType::Node4_Quad:
            return ElemType::Quad4;
        case SimpleElemType::Node8_Quad:
            return ElemType::Quad8;
        case SimpleElemType::Node3_Tri:
            return ElemType::Tri3;
        case SimpleElemType::Node6_Tri:
            return ElemType::Tri6;
        default:
            throw std::runtime_error("Element type is not supported by MeshEngine's mesh format.");
        }
    }

    EgLab::ME::MeshPOD serializeMesh(const SimpleMesh& source)
    {
        auto mesh = EgLab::Common::makeShared<EgLab::ME::Mesh>();
        for (const Node& sourceNode : source.nodes)
        {
            EgLab::ME::Node node;
            node.setId(sourceNode.id);
            EgLab::ME::CoordType coordinates(sourceNode.xyz[0], sourceNode.xyz[1],
                                             sourceNode.xyz[2]);
            node.setXYZ(coordinates);
            mesh->addNode(EgLab::Common::move(node));
        }

        for (const Elem& sourceElem : source.elems)
        {
            EgLab::ME::Elem elem;
            const auto type = convertElemType(sourceElem.type);
            elem.setId(sourceElem.id);
            elem.setType(type);
            const int nodeCount = EgLab::ME::getNodeNumber(type);
            if (nodeCount > maxNodeOfElem)
            {
                throw std::runtime_error("Element node count exceeds maxNodeOfElem.");
            }
            for (int index = 0; index < nodeCount; ++index)
            {
                if (sourceElem.nodes[index] < 0)
                {
                    throw std::runtime_error("Element connectivity contains an invalid node id.");
                }
                elem.setNode(static_cast<EgLab::ME::IdxType>(index), sourceElem.nodes[index]);
            }
            mesh->addElem(EgLab::Common::move(elem));
        }

        return EgLab::ME::MeshToPOD::serialize(mesh);
    }

    bool publishMesh(uint32_t processId, const SimpleMesh& mesh)
    {
        EgLab::ME::MeshPOD pod = serializeMesh(mesh);
        if (pod.data == nullptr || pod.totalDataSize == 0)
        {
            throw std::runtime_error("Mesh serialization returned an empty payload.");
        }

        EgLab::Platform::MeshPODWireHeader wire{};
        wire.nodeNumber = pod.nodeNumber;
        wire.elemNumber = pod.elemNumber;
        wire.nodeIdsOffset = pod.nodeIdsOffset;
        wire.nodeDataOffset = pod.nodeDataOffset;
        wire.elemIdsOffset = pod.elemIdsOffset;
        wire.elemTypesOffset = pod.elemTypesOffset;
        wire.elemDataOffset = pod.elemDataOffset;
        wire.totalDataSize = pod.totalDataSize;

        const uint64_t dataSize64 = sizeof(wire) + static_cast<uint64_t>(pod.totalDataSize);
        if (dataSize64 > std::numeric_limits<uint32_t>::max() - sizeof(uint32_t))
        {
            throw std::runtime_error("Serialized mesh exceeds the shared-memory protocol limit.");
        }

        const uint32_t dataSize = static_cast<uint32_t>(dataSize64);
        EgLab::Common::DynamicArray<uint8_t> packet;
        packet.resize(sizeof(dataSize) + dataSize);
        std::memcpy(packet.data(), &dataSize, sizeof(dataSize));
        std::memcpy(packet.data() + sizeof(dataSize), &wire, sizeof(wire));
        std::memcpy(packet.data() + sizeof(dataSize) + sizeof(wire), pod.data, pod.totalDataSize);

        EgLab::Common::MemoryQueue queue(EgLab::Platform::commandMemoryName(processId), false);
        return queue.push(static_cast<uint32_t>(EgLab::Platform::EventId::UpdateMesh),
                          packet.data(), static_cast<uint32_t>(packet.size()));
    }
} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: command_cross_process_test <EnginePlatform PID>\n";
        return 2;
    }

    uint32_t processId = 0;
    const std::string pidArgument(argv[1]);
    const auto parseResult =
        std::from_chars(pidArgument.data(), pidArgument.data() + pidArgument.size(), processId);
    if (parseResult.ec != std::errc{} ||
        parseResult.ptr != pidArgument.data() + pidArgument.size() || processId == 0)
    {
        std::cerr << "Invalid process ID.\n";
        return 2;
    }
    if (!isEnginePlatformProcess(processId)) return 1;

    try
    {
        bool sent = false;
        for (int attempt = 0; attempt < 50 && !sent; ++attempt)
        {
            try
            {
                sent = publishMesh(processId, makeTestMesh());
            }
            catch (const std::exception&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if (!sent)
        {
            std::cerr << "Could not send UpdateMesh; the target queue is unavailable or full.\n";
            return 1;
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Failed to send UpdateMesh: " << error.what() << '\n';
        return 1;
    }

    std::cout << "UpdateMesh sent to EnginePlatform PID " << processId << ".\n";
    return 0;
}