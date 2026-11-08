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
        UpdateMesh = 200001, // Mesh = 2xxxxx
        MeshImport = 200002,
        MeshDelete = 200003,
        MeshPickInfo = 200004,
        LocateElements = 200005,
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