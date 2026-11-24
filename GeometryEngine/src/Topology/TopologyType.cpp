#include "GeometryEngine/Topology/TopologyType.hpp"

namespace EgLab::GE
{
    const char* TopologyTypeInfo::toString(TopologyType type)
    {
        switch (type)
        {
        case TopologyType::Vertex:
            return "Vertex";
        case TopologyType::Edge:
            return "Edge";
        case TopologyType::Face:
            return "Face";
        case TopologyType::Solid:
            return "Solid";
        }
        return "Unknown";
    }

    int TopologyTypeInfo::getDimension(TopologyType type)
    {
        return isValid(type) ? static_cast<int>(static_cast<std::uint32_t>(type)) : -1;
    }

    bool TopologyTypeInfo::isValid(TopologyType type)
    {
        return static_cast<std::uint32_t>(type) < TOPOLOGY_TYPE_COUNT;
    }

    bool TopologyTypeInfo::canContain(TopologyType parentType, TopologyType childType)
    {
        TopologyType expectedChild = TopologyType::Vertex;
        return tryGetChildType(parentType, expectedChild) && expectedChild == childType;
    }

    bool TopologyTypeInfo::tryGetChildType(TopologyType parentType, TopologyType& childType)
    {
        switch (parentType)
        {
        case TopologyType::Solid:
            childType = TopologyType::Face;
            return true;
        case TopologyType::Face:
            childType = TopologyType::Edge;
            return true;
        case TopologyType::Edge:
            childType = TopologyType::Vertex;
            return true;
        case TopologyType::Vertex:
            break;
        }
        return false;
    }

} // namespace EgLab::GE
