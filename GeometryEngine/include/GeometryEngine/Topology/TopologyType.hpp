#pragma once

#include <cstddef>
#include <cstdint>

#include "GeometryEngine/Common/GeometryAPI.hpp"

namespace EgLab::GE
{
    /// Topological entities ordered by dimension. The numeric value is the dimension,
    /// which lets a topology be traversed with a single index space per entity type.
    enum class TopologyType : std::uint32_t
    {
        Vertex = 0,
        Edge = 1,
        Face = 2,
        Solid = 3
    };

    constexpr std::size_t TOPOLOGY_TYPE_COUNT = 4;

    /// Stateless queries describing the topology hierarchy.
    /// A higher-dimensional entity is the parent of the lower-dimensional entities it bounds.
    class GeometryEngineAPI TopologyTypeInfo
    {
    public:
        static const char* toString(TopologyType type);

        static int getDimension(TopologyType type);

        static bool isValid(TopologyType type);

        /// Returns true when a node of parentType may reference childType as a child.
        static bool canContain(TopologyType parentType, TopologyType childType);

        /// Writes the direct child type of parentType and returns false for leaf types.
        static bool tryGetChildType(TopologyType parentType, TopologyType& childType);
    };

} // namespace EgLab::GE
