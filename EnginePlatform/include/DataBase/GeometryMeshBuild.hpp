#pragma once

#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "DataBase/GeometryData.hpp"

namespace EgLab::GE
{
    class GeometryDocument;
}

namespace EgLab::ME
{
    class Mesh;
}

namespace EgLab::Platform
{
    /// Renderable output of a B-Rep document conversion.
    ///
    /// One mesh carries three topological layers that all reference the same node array, so
    /// every layer is drawn and picked with mesh node indices:
    ///  * faces   -> Tri3 elements,
    ///  * edges   -> polyline node pairs (side table, ME::Elem has no 1D element type),
    ///  * vertices-> single nodes.
    /// The per-layer feature tables keep the mapping element/segment/node -> B-Rep feature.
    struct GeometryMeshBuild
    {
        Common::SharedPtr<ME::Mesh> mesh;
        /// elementFeatures[elemId - 1] owns Tri3 element elemId (faces only).
        Common::DynamicArray<GeometryFeatureRef> elementFeatures;
        /// nodeFeatures[nodeIdx] is the vertex feature rendered by that node, or invalid.
        Common::DynamicArray<GeometryFeatureRef> nodeFeatures;
        /// Edge segments as mesh node index pairs (2 entries per segment).
        Common::DynamicArray<uint32_t> edgeSegmentIndices;
        /// edgeSegmentFeatures[segment] owns edgeSegmentIndices[2 * segment].
        Common::DynamicArray<GeometryFeatureRef> edgeSegmentFeatures;
        uint32_t triangleCount{0};
        uint32_t vertexCount{0};
        uint32_t edgeCount{0};
        uint32_t edgeSegmentCount{0};

        void clear();
    };

    /// Converts a GeometryEngine document into a renderable ME::Mesh plus the topology tables
    /// consumed by GeometryData, GeometryMeshSelector and RenderDataManager.

} // namespace EgLab::Platform
