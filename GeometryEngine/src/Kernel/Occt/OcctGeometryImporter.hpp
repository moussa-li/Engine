#pragma once

#include "GeometryEngine/Kernel/GeometryImportOptions.hpp"
#include "GeometryEngine/Kernel/IGeometryImporter.hpp"

namespace EgLab::GE
{
    /// Adapter between the abstract importer contract and OpenCASCADE.
    ///
    /// All OCCT headers are confined to the implementation file of this class.
    class OcctGeometryImporter : public IGeometryImporter
    {
    public:
        explicit OcctGeometryImporter(const GeometryImportOptions& options);

        Common::String getName() const override;

        GeometryImportReport importFile(const Common::String& filePath,
                                        GeometryDocument& document) const override;

    private:
        GeometryImportOptions _options;
    };

} // namespace EgLab::GE
