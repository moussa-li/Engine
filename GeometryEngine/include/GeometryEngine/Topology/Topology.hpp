#pragma once

#include "Common/Return.hpp"
#include "Common/String.hpp"
#include "Common/UniquePtr.hpp"
#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Common/GeometryTypes.hpp"
#include "GeometryEngine/Topology/TopologyNode.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"

namespace EgLab::GE
{
    /// Owns the topological graph of a model.
    ///
    /// Entities are stored per type, so an index is only meaningful together with its type.
    /// Indices are stable while nodes are only appended; clear() invalidates every index.
    class GeometryEngineAPI Topology
    {
    public:
        Topology();

        ~Topology();

        Topology(const Topology&) = delete;

        Topology& operator=(const Topology&) = delete;

        /// Appends a node and assigns the next free id. Returns INVALID_IDX on failure.
        IdxType createNode(TopologyType type, const Common::String& name);

        /// Appends a node with an explicit id. Returns INVALID_IDX when the id is taken.
        IdxType createNode(TopologyType type, IdType id, const Common::String& name);

        /// Registers childIndex as a child of parentIndex and the reverse parent link.
        Common::Return link(TopologyType parentType, IdxType parentIndex, TopologyType childType,
                            IdxType childIndex);

        std::size_t getNodeCount(TopologyType type) const;

        TopologyNode* getNode(TopologyType type, IdxType index);

        const TopologyNode* getNode(TopologyType type, IdxType index) const;

        IdxType findNodeIndex(TopologyType type, IdType id) const;

        void clear();

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };

} // namespace EgLab::GE
