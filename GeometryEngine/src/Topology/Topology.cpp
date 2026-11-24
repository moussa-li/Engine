#include "GeometryEngine/Topology/Topology.hpp"

#include "Common/HashMap.hpp"

namespace EgLab::GE
{
    class Topology::Impl
    {
    public:
        Common::DynamicArray<TopologyNode>& nodes(TopologyType type)
        {
            switch (type)
            {
            case TopologyType::Vertex:
                return vertices;
            case TopologyType::Edge:
                return edges;
            case TopologyType::Face:
                return faces;
            case TopologyType::Solid:
                return solids;
            }
            return vertices;
        }

        const Common::DynamicArray<TopologyNode>& nodes(TopologyType type) const
        {
            switch (type)
            {
            case TopologyType::Vertex:
                return vertices;
            case TopologyType::Edge:
                return edges;
            case TopologyType::Face:
                return faces;
            case TopologyType::Solid:
                return solids;
            }
            return vertices;
        }

        Common::HashMap<IdType, IdxType>& ids(TopologyType type)
        {
            switch (type)
            {
            case TopologyType::Vertex:
                return vertexIds;
            case TopologyType::Edge:
                return edgeIds;
            case TopologyType::Face:
                return faceIds;
            case TopologyType::Solid:
                return solidIds;
            }
            return vertexIds;
        }

        const Common::HashMap<IdType, IdxType>& ids(TopologyType type) const
        {
            switch (type)
            {
            case TopologyType::Vertex:
                return vertexIds;
            case TopologyType::Edge:
                return edgeIds;
            case TopologyType::Face:
                return faceIds;
            case TopologyType::Solid:
                return solidIds;
            }
            return vertexIds;
        }

        void clear()
        {
            vertices.clear();
            edges.clear();
            faces.clear();
            solids.clear();
            vertexIds.clear();
            edgeIds.clear();
            faceIds.clear();
            solidIds.clear();
        }

        Common::DynamicArray<TopologyNode> vertices;
        Common::DynamicArray<TopologyNode> edges;
        Common::DynamicArray<TopologyNode> faces;
        Common::DynamicArray<TopologyNode> solids;
        Common::HashMap<IdType, IdxType> vertexIds;
        Common::HashMap<IdType, IdxType> edgeIds;
        Common::HashMap<IdType, IdxType> faceIds;
        Common::HashMap<IdType, IdxType> solidIds;
    };

    Topology::Topology() : _impl(Common::makeUnique<Impl>())
    {
    }

    Topology::~Topology() = default;

    IdxType Topology::createNode(TopologyType type, const Common::String& name)
    {
        if (!TopologyTypeInfo::isValid(type)) return INVALID_IDX;

        const auto& storage = _impl->nodes(type);
        const auto& ids = _impl->ids(type);
        IdType id = static_cast<IdType>(storage.size()) + 1;
        while (ids.find(id) != ids.end())
        {
            ++id;
        }
        return createNode(type, id, name);
    }

    IdxType Topology::createNode(TopologyType type, IdType id, const Common::String& name)
    {
        if (!TopologyTypeInfo::isValid(type) || id == INVALID_ID) return INVALID_IDX;

        auto& storage = _impl->nodes(type);
        auto& ids = _impl->ids(type);
        if (ids.find(id) != ids.end()) return INVALID_IDX;

        const auto index = static_cast<IdxType>(storage.size());
        storage.pushBack(TopologyNode(id, name, type));
        ids.insert(Common::make_pair(id, index));
        return index;
    }

    Common::Return Topology::link(TopologyType parentType, IdxType parentIndex, TopologyType childType,
                                  IdxType childIndex)
    {
        if (!TopologyTypeInfo::canContain(parentType, childType)) return Common::Return::BadInput;

        TopologyNode* parent = getNode(parentType, parentIndex);
        TopologyNode* child = getNode(childType, childIndex);
        if (parent == nullptr || child == nullptr) return Common::Return::BadInput;

        parent->addChild(childIndex);
        child->addParent(parentIndex);
        return Common::Return::Succeed;
    }

    std::size_t Topology::getNodeCount(TopologyType type) const
    {
        return TopologyTypeInfo::isValid(type) ? _impl->nodes(type).size() : 0;
    }

    TopologyNode* Topology::getNode(TopologyType type, IdxType index)
    {
        if (!TopologyTypeInfo::isValid(type)) return nullptr;
        auto& storage = _impl->nodes(type);
        return index < storage.size() ? &storage[index] : nullptr;
    }

    const TopologyNode* Topology::getNode(TopologyType type, IdxType index) const
    {
        if (!TopologyTypeInfo::isValid(type)) return nullptr;
        const auto& storage = _impl->nodes(type);
        return index < storage.size() ? &storage[index] : nullptr;
    }

    IdxType Topology::findNodeIndex(TopologyType type, IdType id) const
    {
        if (!TopologyTypeInfo::isValid(type)) return INVALID_IDX;
        const auto& ids = _impl->ids(type);
        const auto found = ids.find(id);
        return found == ids.end() ? INVALID_IDX : (*found).second;
    }

    void Topology::clear()
    {
        _impl->clear();
    }

} // namespace EgLab::GE
