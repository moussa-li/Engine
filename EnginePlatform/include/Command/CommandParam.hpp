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

    static_assert(std::is_trivially_copyable<ShowPanelParam>::value,
                  "ShowPanelParam must be trivially copyable");
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

} // namespace EgLab::Platform