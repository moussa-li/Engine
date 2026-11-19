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

} // namespace EgLab::Platform