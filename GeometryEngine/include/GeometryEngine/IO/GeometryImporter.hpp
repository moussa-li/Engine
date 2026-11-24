#pragma once

#include "Common/String.hpp"
#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Common/GeometryImportReport.hpp"
#include "GeometryEngine/Kernel/GeometryImportOptions.hpp"

namespace EgLab::GE
{
    class GeometryDocument;

    /// Kernel-independent entry point for importing geometry files.
    ///
    /// The importer resolves the requested kernel through GeometryKernelRegistry and lets the
    /// adapter fill the document, so callers never touch a third-party kernel directly.
    class GeometryEngineAPI GeometryImporter
    {
    public:
        GeometryImporter();

        explicit GeometryImporter(const Common::String& kernelName);

        GeometryImporter(const Common::String& kernelName, const GeometryImportOptions& options);

        const Common::String& getKernelName() const;

        void setKernelName(const Common::String& kernelName);

        const GeometryImportOptions& getOptions() const;

        void setOptions(const GeometryImportOptions& options);

        GeometryImportReport importFile(const Common::String& filePath, GeometryDocument& document) const;

    private:
        Common::String _kernelName;
        GeometryImportOptions _options;
    };

} // namespace EgLab::GE
