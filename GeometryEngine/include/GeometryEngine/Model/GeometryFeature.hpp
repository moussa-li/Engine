#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/String.hpp"
#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Common/GeometryTypes.hpp"
#include "GeometryEngine/Geometry/Geometry.hpp"
#include "GeometryEngine/Topology/TopologyNode.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"

namespace EgLab::GE
{
    /// Read-only view that binds one topology node to its geometry.
    ///
    /// The view is a handle into a GeometryDocument: it stays valid until the document is
    /// cleared or new features are appended to the topology.
    class GeometryEngineAPI GeometryFeature
    {
    public:
        GeometryFeature();

        GeometryFeature(const TopologyNode& node, const Geometry& geometry);

        bool isValid() const;

        IdType getId() const;

        const Common::String& getName() const;

        TopologyType getType() const;

        const Common::DynamicArray<IdxType>& getChildren() const;

        const Common::DynamicArray<IdxType>& getParents() const;

        const Geometry& getGeometry() const;

        /// Renderable nodes/elements of this feature.
        const Tessellation& getMesh() const;

    private:
        const TopologyNode* _node{nullptr};
        const Geometry* _geometry{nullptr};
    };

} // namespace EgLab::GE
