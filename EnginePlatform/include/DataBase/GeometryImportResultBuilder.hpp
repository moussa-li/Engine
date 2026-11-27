#pragma once

#include "DataBase/GeometryDataResultPOD.hpp"

namespace EgLab::Platform
{
    class GeometryData;
    struct GeometryMeshBuild;

    /// Builds the GeometryImportResult payload shared by the commands that turn a B-Rep
    /// document into a GeometryData node (file import and buffer update).
    class GeometryImportResultBuilder
    {
    public:
        /// Result for a rejected request; the counters and the node id stay zero.
        static GeometryImportResult makeFailure(uint32_t status);

        /// Result for a published geometry node; fills the topology counts, the Tri3 count,
        /// the bounding box and the data node id.
        static GeometryImportResult makeSuccess(const GeometryData& data,
                                                const GeometryMeshBuild& build);
    };
} // namespace EgLab::Platform
