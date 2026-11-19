#pragma once

#include <cstdint>
#include <type_traits>

#include "Command/CommandParam.hpp"
#include "Common/Protocol.hpp"

namespace EgLab::Platform
{
    constexpr uint32_t MESH_RESULT_MAGIC = 0x4D534844; // 'MSHD'
    constexpr uint32_t MESH_RESULT_VERSION = 1;
    constexpr size_t MAX_ELEM_TYPES_PER_MESH = 10;
    constexpr size_t MAX_MESH_RESULT_ENTRIES = 256;
    constexpr size_t MAX_DATA_TREE_ENTRIES = 256;   // ListDataTreeNodes 单次返回上限

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

    // ===== 精确查询（node id / elem id 粒度）结果 =====
    // 所有精确查询结果复用同一魔数与版本号，Python 侧按命令(eventId)区分结构。

    // 精确查询结果公共头（32 字节）。
    struct MeshQueryResultHeader
    {
        uint32_t magic;     // 0  'MSHD'
        uint32_t version;   // 4  =1
        uint32_t status;    // 8  0=ok 1=网格不存在/非 MeshData 2=输入非法
        uint32_t total;     // 12 总条目数（用于分页）
        uint32_t offset;    // 16 本次起始偏移
        uint32_t returned;  // 20 本次实际返回条目数
        uint32_t reserved[2]; // 24
    };

    // 数据树节点清单条目（96 字节）。
    struct DataTreeNodeEntry
    {
        uint64_t dataNodeId; // 0
        char name[64];       // 8
        uint32_t kind;       // 72 0=DataNode 1=MeshData
        uint32_t reserved;   // 76
        uint64_t nodeCount;  // 80 （MeshData 时有效，否则 0）
        uint64_t elemCount;  // 88 （MeshData 时有效，否则 0）
    };

    struct ListDataTreeResult
    { // 32 + 256*96 = 24608 字节
        MeshQueryResultHeader header;
        DataTreeNodeEntry entries[MAX_DATA_TREE_ENTRIES];
    };

    struct MeshIdsResult
    { // 32 + 4096*4 = 16416 字节
        MeshQueryResultHeader header;
        uint32_t ids[MAX_QUERY_PAGE_IDS];
    };

    struct NodeXYZEntry
    { // 16 字节
        uint32_t id;
        float xyz[3];
    };

    struct NodesXYZResult
    { // 32 + 256*16 = 4128 字节
        MeshQueryResultHeader header;
        NodeXYZEntry nodes[MAX_QUERY_IDS];
    };

    struct ElementTopoEntry
    { // 92 字节
        uint32_t id;
        uint32_t type;      // ME::ElemType 枚举值（Tri3=0x203, Quad4=0x204, Tet4=0x304 ...）
        uint32_t nodeCount;
        uint32_t nodeIds[MAX_ELEM_NODE_COUNT];
    };

    struct ElementsTopoResult
    { // 32 + 256*92 = 23584 字节
        MeshQueryResultHeader header;
        ElementTopoEntry elements[MAX_QUERY_IDS];
    };

    static_assert(std::is_trivially_copyable<MeshQueryResultHeader>::value,
                  "MeshQueryResultHeader must be trivially copyable");
    static_assert(sizeof(MeshQueryResultHeader) == 32,
                  "MeshQueryResultHeader must be exactly 32 bytes");
    static_assert(std::is_trivially_copyable<DataTreeNodeEntry>::value,
                  "DataTreeNodeEntry must be trivially copyable");
    static_assert(sizeof(DataTreeNodeEntry) == 96, "DataTreeNodeEntry must be exactly 96 bytes");
    static_assert(std::is_trivially_copyable<ListDataTreeResult>::value,
                  "ListDataTreeResult must be trivially copyable");
    static_assert(sizeof(ListDataTreeResult) == 24608, "ListDataTreeResult must be 24608 bytes");
    static_assert(std::is_trivially_copyable<MeshIdsResult>::value,
                  "MeshIdsResult must be trivially copyable");
    static_assert(sizeof(MeshIdsResult) == 16416, "MeshIdsResult must be 16416 bytes");
    static_assert(std::is_trivially_copyable<NodeXYZEntry>::value,
                  "NodeXYZEntry must be trivially copyable");
    static_assert(sizeof(NodeXYZEntry) == 16, "NodeXYZEntry must be exactly 16 bytes");
    static_assert(std::is_trivially_copyable<NodesXYZResult>::value,
                  "NodesXYZResult must be trivially copyable");
    static_assert(sizeof(NodesXYZResult) == 4128, "NodesXYZResult must be 4128 bytes");
    static_assert(std::is_trivially_copyable<ElementTopoEntry>::value,
                  "ElementTopoEntry must be trivially copyable");
    static_assert(sizeof(ElementTopoEntry) == 92, "ElementTopoEntry must be exactly 92 bytes");
    static_assert(std::is_trivially_copyable<ElementsTopoResult>::value,
                  "ElementsTopoResult must be trivially copyable");
    static_assert(sizeof(ElementsTopoResult) == 23584, "ElementsTopoResult must be 23584 bytes");
    static_assert(sizeof(ListDataTreeResult) <= Common::MAX_PAYLOAD_SIZE,
                  "ListDataTreeResult exceeds the shared-memory payload limit");
    static_assert(sizeof(MeshIdsResult) <= Common::MAX_PAYLOAD_SIZE,
                  "MeshIdsResult exceeds the shared-memory payload limit");
    static_assert(sizeof(NodesXYZResult) <= Common::MAX_PAYLOAD_SIZE,
                  "NodesXYZResult exceeds the shared-memory payload limit");
    static_assert(sizeof(ElementsTopoResult) <= Common::MAX_PAYLOAD_SIZE,
                  "ElementsTopoResult exceeds the shared-memory payload limit");
} // namespace EgLab::Platform
