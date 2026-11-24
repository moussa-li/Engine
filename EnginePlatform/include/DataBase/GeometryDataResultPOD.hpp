#pragma once

#include <cstdint>
#include <type_traits>

namespace EgLab::Platform
{
    // Little-endian "GEOM".
    constexpr uint32_t GEOM_RESULT_MAGIC = 0x4D4F4547;
    constexpr uint32_t GEOM_RESULT_VERSION = 1;

    // 单条消息内最多返回的特征条目数（受共享内存单槽 65536 字节上限约束）。
    // header(32B) + entries*152B <= 65536。
    constexpr size_t GEOM_MAX_FEATURE_ENTRIES = 430;

    // 查询结果公共头，与 MeshQueryResultHeader 同构但 magic 不同。
    struct GeometryResultHeader
    {                          // 32B
        uint32_t magic;        // 0  = 0x4D4F4547
        uint32_t version;      // 4  = 1
        uint32_t status;       // 8  0=ok 1=非GeometryData 2=特征不存在 3=数据不可用 4=类型非法
        uint32_t total;        // 12 匹配总数
        uint32_t offset;       // 16 恒为 0
        uint32_t returned;     // 20 本次实际返回条目数
        uint32_t reserved[2];  // 24
    };

    // 单个几何特征条目，与 Python struct "<IIIIII6d6f56s" 对齐。
    struct GeometryFeatureEntry
    {                            // 152B
        uint32_t featureId;      // 0   GE::IdType
        uint32_t featureType;    // 4   GE::TopologyType: 0=V 1=E 2=F 3=S
        uint32_t geometryType;   // 8   GE::GeometryType: 0=Unknown 1=Point 2=Curve 3=Surface 4=Solid
        uint32_t parentCount;    // 12
        uint32_t childCount;     // 16
        uint32_t elemId;         // 20  命中的渲染 Tri3 elemId；无则 0xFFFFFFFF
        double length;           // 24
        double area;             // 32
        double volume;           // 40
        double x;                // 48  包围盒中心
        double y;                // 56
        double z;                // 64
        float bboxMin[3];        // 72
        float bboxMax[3];        // 84
        char name[56];           // 96  UTF-8, 0 结尾
    };

    // 导入结果，与 Python struct "<IIIIQ6I6f" 对齐。
    struct GeometryImportResult
    {                          // 72B
        uint32_t magic;        // 0  = 0x4D4F4547
        uint32_t version;      // 4  = 1
        uint32_t status;       // 8  0=ok 1=入参/文件 2=导入失败 3=无三角面 4=节点注册失败
        uint32_t flags;        // 12 恒为 0
        uint64_t dataNodeId;   // 16 新建 GeometryData 节点 id；失败为 0
        uint32_t vertexCount;  // 24
        uint32_t edgeCount;    // 28
        uint32_t faceCount;    // 32
        uint32_t solidCount;   // 36
        uint32_t triangleCount;// 40 实际生成的 Tri3 数量
        uint32_t reserved;     // 44 恒为 0
        float bboxMin[3];      // 48
        float bboxMax[3];      // 60
    };

    static_assert(std::is_trivially_copyable<GeometryResultHeader>::value,
                  "GeometryResultHeader must be trivially copyable");
    static_assert(sizeof(GeometryResultHeader) == 32,
                  "GeometryResultHeader must be exactly 32 bytes");
    static_assert(std::is_trivially_copyable<GeometryFeatureEntry>::value,
                  "GeometryFeatureEntry must be trivially copyable");
    static_assert(sizeof(GeometryFeatureEntry) == 152,
                  "GeometryFeatureEntry must be exactly 152 bytes");
    static_assert(std::is_trivially_copyable<GeometryImportResult>::value,
                  "GeometryImportResult must be trivially copyable");
    static_assert(sizeof(GeometryImportResult) == 72,
                  "GeometryImportResult must be exactly 72 bytes");
} // namespace EgLab::Platform
