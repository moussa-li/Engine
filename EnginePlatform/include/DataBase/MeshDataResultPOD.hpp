#pragma once

#include <cstdint>
#include <type_traits>

#include "Common/Protocol.hpp"

namespace EgLab::Platform
{
    constexpr uint32_t MESH_RESULT_MAGIC = 0x4D534844; // 'MSHD'
    constexpr uint32_t MESH_RESULT_VERSION = 1;
    constexpr size_t MAX_ELEM_TYPES_PER_MESH = 10;
    constexpr size_t MAX_MESH_RESULT_ENTRIES = 256;

    struct MeshResultEntry
    { // 200 字节
        uint64_t dataNodeId; // off 0
        char name[64];       // off 8
        uint64_t nodeCount;  // off 72
        uint64_t elemCount;  // off 80
        float bboxMin[3];    // off 88
        float bboxMax[3];    // off 100
        uint32_t elemTypeCount;                            // off 112
        uint32_t elemTypeIds[MAX_ELEM_TYPES_PER_MESH];     // off 116
        uint32_t elemTypeCounts[MAX_ELEM_TYPES_PER_MESH];  // off 156
    };

    struct GetMeshDataResultHeader
    {                       // 64 字节
        uint32_t magic;     // off 0 'MSHD'
        uint32_t version;   // off 4 =1
        uint32_t status;    // off 8 0=ok 1=不存在/非 MeshData
        uint32_t flags;     // off 12 bit0=truncated(条目>256)
        uint32_t meshCount; // off 16
        uint64_t totalNodeCount; // off 24
        uint64_t totalElemCount; // off 32
        float totalBBoxMin[3];    // off 40
        float totalBBoxMax[3];    // off 52
    };

    struct GetMeshDataResult
    { // 64 + 256*200 = 51264 字节
        GetMeshDataResultHeader header;
        MeshResultEntry entries[MAX_MESH_RESULT_ENTRIES];
    };

    static_assert(std::is_trivially_copyable<MeshResultEntry>::value,
                  "MeshResultEntry must be trivially copyable");
    static_assert(sizeof(MeshResultEntry) == 200, "MeshResultEntry must be exactly 200 bytes");
    static_assert(std::is_trivially_copyable<GetMeshDataResultHeader>::value,
                  "GetMeshDataResultHeader must be trivially copyable");
    static_assert(sizeof(GetMeshDataResultHeader) == 64,
                  "GetMeshDataResultHeader must be exactly 64 bytes");
    static_assert(std::is_trivially_copyable<GetMeshDataResult>::value,
                  "GetMeshDataResult must be trivially copyable");
    static_assert(sizeof(GetMeshDataResult) == 51264, "GetMeshDataResult must be 51264 bytes");
    static_assert(sizeof(GetMeshDataResult) <= Common::MAX_PAYLOAD_SIZE,
                  "GetMeshDataResult exceeds the shared-memory payload limit");
} // namespace EgLab::Platform
