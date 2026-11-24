#pragma once

#include "Common/Return.hpp"
#include "Common/String.hpp"
#include "Common/UniquePtr.hpp"
#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Common/GeometryImportReport.hpp"
#include "GeometryEngine/Common/GeometryTypes.hpp"
#include "GeometryEngine/Geometry/Geometry.hpp"
#include "GeometryEngine/Model/GeometryFeature.hpp"
#include "GeometryEngine/Topology/Topology.hpp"

namespace EgLab::GE
{
    /// A geometry document: one topology plus the geometry attached to every topology node.
    class GeometryEngineAPI GeometryDocument
    {
    public:
        GeometryDocument();

        ~GeometryDocument();

        GeometryDocument(const GeometryDocument&) = delete;

        GeometryDocument& operator=(const GeometryDocument&) = delete;

        Topology& getTopology();

        const Topology& getTopology() const;

        /// Appends a topology node together with its (empty) geometry.
        /// Returns INVALID_IDX when the node could not be created.
        IdxType createFeature(TopologyType type, const Common::String& name);

        /// Links a parent feature to a child feature (solid -> face -> edge -> vertex).
        Common::Return linkFeatures(TopologyType parentType, IdxType parentIndex, TopologyType childType,
                                    IdxType childIndex);

        std::size_t getFeatureCount(TopologyType type) const;

        GeometryFeature getFeature(TopologyType type, IdxType index) const;

        Geometry* getGeometry(TopologyType type, IdxType index);

        const Geometry* getGeometry(TopologyType type, IdxType index) const;

        const GeometryImportReport& getLastReport() const;

        void setLastReport(const GeometryImportReport& report);

        void clear();

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };

} // namespace EgLab::GE
