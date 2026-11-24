#pragma once

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
    };

} // namespace EgLab::GE
