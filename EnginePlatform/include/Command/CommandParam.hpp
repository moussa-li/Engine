#pragma once

#include <stddef.h>

#include <cstdint>
#include <type_traits>

#include "Common/Protocol.hpp"

namespace EgLab::Platform
{
    struct ShowPanelParam
    {
        char panelName[64];
    };

    struct HidePanelParam
    {
        char panelName[64];
    };

    struct ImportMeshParam
    {
        char fileDir[128];
    };

    struct UpdateMeshParam
    {
        uint32_t dataSize;
        uint8_t* data;
    };

    struct DeleteMeshParam
    {
        uint64_t dataNodeId;
    };

    struct GetMeshDataParam
    {
        uint64_t dataNodeId; // 0 = 整个场景汇总
    };

    // ===== 网格数据精确查询（按 node id / elem id 粒度） =====
    // 单个单元的最多节点数（Hex20 = 20，这是单元类型的物理上限，不是请求数量限制）。
    constexpr size_t MAX_ELEM_NODE_COUNT = 20;
    // 查询时网格名称的最大长度（含结尾 '\0'）。
    constexpr size_t MESH_QUERY_NAME_CAPACITY = 64;

    // 统一的网格定位：dataNodeId=0 时按 name 查找，二者都为空则输入非法。
    // 不设分页/数量上限：一次调用返回全部结果。
    struct MeshQueryParam
    {
        uint64_t dataNodeId; // 0 = 按 name 查找
        char name[MESH_QUERY_NAME_CAPACITY];
    };

    // GetNodesXYZ：给定网格 + 一批节点 id，返回每个节点的 xyz。
    // nodeIds 由命令动态分配并持有，count 为其数量；无数量上限。
    struct MeshNodesQueryParam
    {
        uint64_t dataNodeId; // 0 = 按 name 查找
        char name[MESH_QUERY_NAME_CAPACITY];
        uint32_t count;
        uint32_t reserved;
        uint32_t* nodeIds;
    };

    // GetElementsTopology：给定网格 + 一批单元 id，返回每个单元的类型与拓扑节点。
    // elemIds 由命令动态分配并持有，count 为其数量；无数量上限。
    struct MeshElementsQueryParam
    {
        uint64_t dataNodeId; // 0 = 按 name 查找
        char name[MESH_QUERY_NAME_CAPACITY];
        uint32_t count;
        uint32_t reserved;
        uint32_t* elemIds;
    };

    // ===== Geometry（几何引擎导入与特征查询） =====
    constexpr size_t GEOMETRY_FILE_PATH_CAPACITY = 512;
    constexpr size_t GEOMETRY_KERNEL_NAME_CAPACITY = 64;
    constexpr size_t GEOMETRY_QUERY_NAME_CAPACITY = 64;

    // ImportGeometry：从 STEP/BREP 文件导入 B-Rep 为 GeometryData。
    struct ImportGeometryParam
    {
        char filePath[GEOMETRY_FILE_PATH_CAPACITY];     // off 0, 512B, UTF-8, 0 结尾
        char kernelName[GEOMETRY_KERNEL_NAME_CAPACITY]; // off 512, 64B; 空串 => 默认内核
    };

    // GetGeometryInfo：featureType 用 GE::TopologyType 数值(0=V 1=E 2=F 3=S)，
    // 0xFFFFFFFF 表示改由 elemId 反查；elemId 非 0 时优先于 featureType/featureId。
    struct GeometryQueryParam
    {
        uint64_t dataNodeId; // off 0; 0 = 按 name 查找
        char name[GEOMETRY_QUERY_NAME_CAPACITY]; // off 8
        uint32_t featureType; // off 72
        uint32_t featureId;   // off 76; feature index
        uint32_t elemId;      // off 80; 渲染 Tri3 elemId；非 0 时优先
        uint32_t reserved;    // off 84; 恒为 0
    };

    // ListGeometryFeatures：featureType 0xFFFFFFFF 表示全部类型；分页由 offset/limit 控制。
    struct GeometryFeaturesParam
    {
        uint64_t dataNodeId; // off 0; 0 = 按 name 查找
        char name[GEOMETRY_QUERY_NAME_CAPACITY]; // off 8
        uint32_t featureType; // off 72; 0xFFFFFFFF = 全部
        uint32_t offset;      // off 76
        uint32_t limit;       // off 80; 0 = 全部
        uint32_t reserved;    // off 84; 恒为 0
    };

    // SetGeometryRenderMode：几何渲染模式。0=网格（面+三角线框），1=面外框（面+面边界），
    // 2=仅几何线（不渲染面）。作用于场景内全部导入的 B-Rep 几何模型。
    struct GeometryRenderModeParam
    {
        uint32_t mode;     // off 0; 0=Mesh 1=FaceOutline 2=EdgeOnly
        uint32_t reserved; // off 4; 恒为 0
    };

    // SetGeometryPickFilter：几何拾取/框选的特征类型过滤（bit0=Vertex bit1=Edge
    // bit2=Face bit3=Solid）。作用于场景内全部导入的 B-Rep 几何模型。
    struct GeometryPickFilterParam
    {
        uint32_t mask;     // off 0; bit0=Vertex bit1=Edge bit2=Face bit3=Solid
        uint32_t reserved; // off 4; 恒为 0
    };

    // SetGeometryVertexDisplay：几何顶点标记（只显示 ref 到 Vertex 特征的节点）显隐。
    struct GeometryVertexDisplayParam
    {
        uint32_t visible;  // off 0; 0=隐藏 1=显示
        uint32_t reserved; // off 4; 恒为 0
    };

    // SetGeometryZebra：几何面斑马纹（等照度）分析显示。作用于场景内全部 B-Rep 几何模型。
    struct GeometryZebraParam
    {
        uint32_t enabled;        // off 0; 0=关闭 1=开启
        float frequency;         // off 4; 条纹密度(2~40)，<=0 或非数时使用默认值 8
        float lightDirection[3]; // off 8; 虚拟光源方向(模型空间)，零向量时使用默认值 (0,0,1)。
                                 // 斑马纹由该方向反射得到，与相机无关。
    };

    // SetGeometryUv：几何面曲面 UV 显示。作用于场景内全部 B-Rep 几何模型。
    struct GeometryUvParam
    {
        uint32_t enabled;  // off 0; 0=关闭 1=显示曲面 UV
        uint32_t reserved; // off 4; 恒为 0
    };

    // ===== SetCamera =====
    enum class CameraMode : uint32_t
    {
        Orbit = 0,
        Fps = 1,
    };

    constexpr uint32_t CAMERA_FIELD_POSITION = 1u << 0;
    constexpr uint32_t CAMERA_FIELD_TARGET = 1u << 1;
    constexpr uint32_t CAMERA_FIELD_FOV = 1u << 2;
    constexpr uint32_t CAMERA_FIELD_DURATION = 1u << 3;

    struct SetCameraParam
    {
        uint32_t mode;      // CameraMode::Orbit/Fps
        uint32_t fieldMask; // CAMERA_FIELD_* 位标志
        float position[3];
        float target[3];
        float fov;      // 垂直视场角(度) → RE::Camera::setZoom(fov)
        float duration; // 秒，<=0 立即生效，缺省 0.45f
    };

    // Maximum number of meshes that can be highlighted by a single Highlight command.
    constexpr size_t MAX_HIGHLIGHT_DATA_NODE_COUNT = 64;
    // Maximum number of element ids that can be located by a single LocateElements command.
    constexpr size_t MAX_LOCATE_ELEMENT_COUNT = 512;

    // Highlight: replaces the highlighted mesh set, identified by data node ids.
    struct HighlightParam
    {
        uint32_t count;
        uint64_t dataNodeIds[MAX_HIGHLIGHT_DATA_NODE_COUNT];
    };

    // Locate: highlights the given elements of one mesh and makes all other faces transparent.
    struct LocateElementsParam
    {
        uint64_t dataNodeId;
        uint32_t count;
        uint32_t elementIds[MAX_LOCATE_ELEMENT_COUNT];
    };

    // Visibility: shows or hides elements of one mesh. The mode field selects the operation:
    // 0 = hide the given ids, 1 = show the given ids, 2 = show only the given ids (hide the
    // rest), 3 = hide only the given ids (show the rest). For modes 1/3 an empty id list shows
    // all elements; for mode 2 an empty id list hides every element. elementIds is dynamically
    // allocated and owned by the command.
    struct ElementsVisibilityParam
    {
        uint64_t dataNodeId;
        uint32_t mode;
        uint32_t count;
        uint32_t* elementIds;
    };

    // Node visibility: shows or hides nodes of one mesh. The mode field follows the element
    // visibility semantics: 0 = hide, 1 = show, 2 = show only, 3 = hide only. For modes 1/3 an
    // empty id list shows all nodes; for mode 2 an empty id list hides every node. nodeIds is
    // dynamically allocated and owned by the command.
    struct NodeVisibilityParam
    {
        uint64_t dataNodeId;
        uint32_t mode;
        uint32_t count;
        uint32_t* nodeIds;
    };

    // ===== SetReferencePlane =====
    // Field bits select which reference-plane properties the command updates.
    constexpr uint32_t REFERENCE_PLANE_FIELD_ENABLED = 1u << 0;
    constexpr uint32_t REFERENCE_PLANE_FIELD_SHOW_GRID = 1u << 1;
    constexpr uint32_t REFERENCE_PLANE_FIELD_PLACEMENT = 1u << 2;
    constexpr uint32_t REFERENCE_PLANE_FIELD_SCALE = 1u << 3;
    constexpr uint32_t REFERENCE_PLANE_FIELD_COLOR_X = 1u << 4;
    constexpr uint32_t REFERENCE_PLANE_FIELD_COLOR_Y = 1u << 5;
    constexpr uint32_t REFERENCE_PLANE_FIELD_COLOR_Z = 1u << 6;
    constexpr uint32_t REFERENCE_PLANE_FIELD_ALL =
        REFERENCE_PLANE_FIELD_ENABLED | REFERENCE_PLANE_FIELD_SHOW_GRID |
        REFERENCE_PLANE_FIELD_PLACEMENT | REFERENCE_PLANE_FIELD_SCALE |
        REFERENCE_PLANE_FIELD_COLOR_X | REFERENCE_PLANE_FIELD_COLOR_Y |
        REFERENCE_PLANE_FIELD_COLOR_Z;

    // Reference plane placement values shared with RE::ReferencePlanePlacement.
    constexpr uint32_t REFERENCE_PLANE_PLACEMENT_CENTER = 0;
    constexpr uint32_t REFERENCE_PLANE_PLACEMENT_BACK_FACES = 1;

    // Applies a partial reference-plane update. Only fields selected by fieldMask are read.
    // colors[0] = YZ plane (X normal), [1] = XZ plane (Y normal), [2] = XY plane (Z normal),
    // each stored as r, g, b, a.
    struct SetReferencePlaneParam
    {
        uint32_t fieldMask;
        uint32_t enabled;
        uint32_t showGrid;
        uint32_t placement;
        float scale;
        float colors[3][4];
    };

    static_assert(std::is_trivially_copyable<ShowPanelParam>::value,
                  "ShowPanelParam must be trivially copyable");
    static_assert(std::is_trivially_copyable<HidePanelParam>::value,
                  "HidePanelParam must be trivially copyable");
    static_assert(sizeof(HidePanelParam) == 64, "HidePanelParam must be exactly 64 bytes");
    static_assert(std::is_trivially_copyable<SetCameraParam>::value,
                  "SetCameraParam must be trivially copyable");
    static_assert(sizeof(SetCameraParam) == 40, "SetCameraParam must be exactly 40 bytes");
    static_assert(std::is_trivially_copyable<GetMeshDataParam>::value,
                  "GetMeshDataParam must be trivially copyable");
    static_assert(sizeof(GetMeshDataParam) == 8, "GetMeshDataParam must be exactly 8 bytes");
    static_assert(std::is_trivially_copyable<ImportMeshParam>::value,
                  "ImportMeshParam must be trivially copyable");
    static_assert(std::is_trivially_copyable<UpdateMeshParam>::value,
                  "UpdateMeshParam must be trivially copyable");
    static_assert(std::is_trivially_copyable<DeleteMeshParam>::value,
                  "DeleteMeshParam must be trivially copyable");
    static_assert(std::is_trivially_copyable<HighlightParam>::value,
                  "HighlightParam must be trivially copyable");
    static_assert(std::is_trivially_copyable<LocateElementsParam>::value,
                  "LocateElementsParam must be trivially copyable");
    static_assert(std::is_trivially_copyable<ElementsVisibilityParam>::value,
                  "ElementsVisibilityParam must be trivially copyable");
    static_assert(std::is_trivially_copyable<NodeVisibilityParam>::value,
                  "NodeVisibilityParam must be trivially copyable");
    static_assert(std::is_trivially_copyable<SetReferencePlaneParam>::value,
                  "SetReferencePlaneParam must be trivially copyable");
    static_assert(sizeof(SetReferencePlaneParam) == 68,
                  "SetReferencePlaneParam must be exactly 68 bytes");
    static_assert(std::is_trivially_copyable<MeshQueryParam>::value,
                  "MeshQueryParam must be trivially copyable");
    static_assert(sizeof(MeshQueryParam) == 72, "MeshQueryParam must be exactly 72 bytes");
    static_assert(std::is_trivially_copyable<MeshNodesQueryParam>::value,
                  "MeshNodesQueryParam must be trivially copyable");
    static_assert(std::is_trivially_copyable<MeshElementsQueryParam>::value,
                  "MeshElementsQueryParam must be trivially copyable");
    static_assert(std::is_trivially_copyable<ImportGeometryParam>::value,
                  "ImportGeometryParam must be trivially copyable");
    static_assert(sizeof(ImportGeometryParam) == 576,
                  "ImportGeometryParam must be exactly 576 bytes");
    static_assert(std::is_trivially_copyable<GeometryQueryParam>::value,
                  "GeometryQueryParam must be trivially copyable");
    static_assert(sizeof(GeometryQueryParam) == 88,
                  "GeometryQueryParam must be exactly 88 bytes");
    static_assert(std::is_trivially_copyable<GeometryFeaturesParam>::value,
                  "GeometryFeaturesParam must be trivially copyable");
    static_assert(sizeof(GeometryFeaturesParam) == 88,
                  "GeometryFeaturesParam must be exactly 88 bytes");
    static_assert(std::is_trivially_copyable<GeometryRenderModeParam>::value,
                  "GeometryRenderModeParam must be trivially copyable");
    static_assert(sizeof(GeometryRenderModeParam) == 8,
                  "GeometryRenderModeParam must be exactly 8 bytes");
    static_assert(std::is_trivially_copyable<GeometryPickFilterParam>::value,
                  "GeometryPickFilterParam must be trivially copyable");
    static_assert(sizeof(GeometryPickFilterParam) == 8,
                  "GeometryPickFilterParam must be exactly 8 bytes");
    static_assert(std::is_trivially_copyable<GeometryVertexDisplayParam>::value,
                  "GeometryVertexDisplayParam must be trivially copyable");
    static_assert(sizeof(GeometryVertexDisplayParam) == 8,
                  "GeometryVertexDisplayParam must be exactly 8 bytes");
    static_assert(std::is_trivially_copyable<GeometryZebraParam>::value,
                  "GeometryZebraParam must be trivially copyable");
    static_assert(sizeof(GeometryZebraParam) == 20,
                  "GeometryZebraParam must be exactly 20 bytes");
    static_assert(std::is_trivially_copyable<GeometryUvParam>::value,
                  "GeometryUvParam must be trivially copyable");
    static_assert(sizeof(GeometryUvParam) == 8, "GeometryUvParam must be exactly 8 bytes");

} // namespace EgLab::Platform