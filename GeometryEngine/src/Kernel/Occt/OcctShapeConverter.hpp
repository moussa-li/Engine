#pragma once

#include "Common/Return.hpp"
#include "Common/UniquePtr.hpp"
#include "GeometryEngine/Common/GeometryImportReport.hpp"
#include "GeometryEngine/Kernel/GeometryImportOptions.hpp"

class TopoDS_Shape;

namespace EgLab::GE
{
    class GeometryDocument;

    /// Converts one OCCT shape into the neutral topology/geometry document.
    ///
    /// Internal to the OCCT adapter: native shapes never leave this translation unit boundary.
    class OcctShapeConverter
    {
    public:
        explicit OcctShapeConverter(const GeometryImportOptions& options);

        ~OcctShapeConverter();

        OcctShapeConverter(const OcctShapeConverter&) = delete;

        OcctShapeConverter& operator=(const OcctShapeConverter&) = delete;

        Common::Return convert(const TopoDS_Shape& shape, GeometryDocument& document,
                               GeometryImportReport& report);

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };

} // namespace EgLab::GE
