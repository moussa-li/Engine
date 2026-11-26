#pragma once

#include <cstdint>
#include <functional>
#include <type_traits>

#include "Common/HashTable.hpp"

namespace EgLab::Platform
{

    enum class EventId : int32_t
    {
        Unknow = 0,
        ShowPanel = 100001,
        FitView = 100002,
        Highlight = 100003,
        HidePanel = 100004,
        UpdateMesh = 200001, // Mesh = 2xxxxx
        MeshImport = 200002,
        MeshDelete = 200003,
        MeshPickInfo = 200004,
        LocateElements = 200005,
        ElementsVisibility = 200006,
        NodeVisibility = 200007,
        SelectionChanged = 200008,
        GetMeshData = 200010, // 200009 is reserved for CreateMesh
        ListDataTreeNodes = 200011, // 数据树节点清单（id + 名称 + 类型）
        GetMeshNodes = 200012,      // 网格节点 id 列表（分页）
        GetMeshElements = 200013,   // 网格单元 id 列表（分页）
        GetNodesXYZ = 200014,       // 按节点 id 查询坐标
        GetElementsTopology = 200015, // 按单元 id 查询拓扑（类型 + 节点 id）
        GeometryImport = 200020,    // 导入 .stp/.step/.brp/.brep 为 GeometryData
        GetGeometryInfo = 200021,   // 查询单个几何特征信息
        ListGeometryFeatures = 200022, // 列出 GeometryData 的几何特征
        SetGeometryRenderMode = 200023, // 设置几何渲染模式（网格/面外框/仅几何线）
        SetGeometryPickFilter = 200024, // 设置几何拾取/框选的特征类型过滤
        SetGeometryVertexDisplay = 200025, // 显示/隐藏几何顶点标记
        SetGeometryZebra = 200026,         // 几何面斑马纹（等照度）分析显示
        CameraPan = 300001, // Camera = 3xxxxx
        CameraRotate = 300002,
        CameraRotateLocal = 300003,
        CameraZoom = 300004,
        SetCamera = 300005,
        SetReferencePlane = 400001, // Appearance = 4xxxxx
    };
}

namespace EgLab::Common
{
    // Allows EventId to be used directly as a Common::HashMap key (for example the shortcut table).
    template <>
    struct hash<EgLab::Platform::EventId>
    {
        size_t operator()(const EgLab::Platform::EventId& eventId) const
        {
            using UnderlyingType = std::underlying_type_t<EgLab::Platform::EventId>;
            return std::hash<UnderlyingType>{}(static_cast<UnderlyingType>(eventId));
        }
    };
} // namespace EgLab::Common