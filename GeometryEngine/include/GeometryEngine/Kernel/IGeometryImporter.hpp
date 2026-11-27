#pragma once

#include <cstddef>

#include "Common/String.hpp"
#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Common/GeometryImportReport.hpp"

namespace EgLab::GE
{
    class GeometryDocument;

    /// Abstract product of a geometry kernel factory.
    ///
    /// Every adapter converts its native shape representation into the neutral
    /// Topology/Geometry model of the document; no kernel type crosses this interface.
    class GeometryEngineAPI IGeometryImporter
    {
    public:
        virtual ~IGeometryImporter() = default;

        virtual Common::String getName() const = 0;

        virtual GeometryImportReport importFile(const Common::String& filePath,
                                                GeometryDocument& document) const = 0;

        /// Imports a shape serialized in memory by the adapter's native writer (for OCCT:
        /// BinTools binary BREP or BRepTools ASCII BREP). The bytes never escape the adapter,
        /// so callers stay independent of the kernel's serialization format.
        virtual GeometryImportReport importBuffer(const void* data, std::size_t size,
                                                  GeometryDocument& document) const = 0;
    };

} // namespace EgLab::GE
