#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/String.hpp"
#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Common/GeometryTypes.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"

namespace EgLab::GE
{
    /// One topological entity (vertex, edge, face or solid).
    ///
    /// Parent/child entries are indices into the array of the corresponding entity type:
    /// a solid references faces, a face references edges, an edge references vertices.
    /// A vertex keeps the edges it belongs to in its parent list.
    class GeometryEngineAPI TopologyNode
    {
    public:
        TopologyNode();

        TopologyNode(IdType id, const Common::String& name, TopologyType type);

        IdType getId() const;

        void setId(IdType id);

        const Common::String& getName() const;

        void setName(const Common::String& name);

        TopologyType getType() const;

        const Common::DynamicArray<IdxType>& getChildren() const;

        const Common::DynamicArray<IdxType>& getParents() const;

        bool hasChild(IdxType childIndex) const;

        bool hasParent(IdxType parentIndex) const;

        void addChild(IdxType childIndex);

        void addParent(IdxType parentIndex);

        void clearRelations();

    private:
        IdType _id{INVALID_ID};
        Common::String _name;
        TopologyType _type{TopologyType::Vertex};
        Common::DynamicArray<IdxType> _children;
        Common::DynamicArray<IdxType> _parents;
    };

} // namespace EgLab::GE
