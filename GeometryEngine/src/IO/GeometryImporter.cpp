#include "GeometryEngine/IO/GeometryImporter.hpp"

#include "GeometryEngine/Kernel/GeometryKernelRegistry.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"

namespace EgLab::GE
{
    GeometryImporter::GeometryImporter()
        : _kernelName(GeometryKernelRegistry::getDefaultKernelName())
    {
    }

    GeometryImporter::GeometryImporter(const Common::String& kernelName) : _kernelName(kernelName)
    {
    }

    GeometryImporter::GeometryImporter(const Common::String& kernelName,
                                       const GeometryImportOptions& options)
        : _kernelName(kernelName), _options(options)
    {
    }

    const Common::String& GeometryImporter::getKernelName() const
    {
        return _kernelName;
    }

    void GeometryImporter::setKernelName(const Common::String& kernelName)
    {
        _kernelName = kernelName;
    }

    const GeometryImportOptions& GeometryImporter::getOptions() const
    {
        return _options;
    }

    void GeometryImporter::setOptions(const GeometryImportOptions& options)
    {
        _options = options;
    }

    GeometryImportReport GeometryImporter::importFile(const Common::String& filePath,
                                                      GeometryDocument& document) const
    {
        document.clear();

        Common::String kernelName = _kernelName;
        if (kernelName.size() == 0)
        {
            kernelName = GeometryKernelRegistry::getDefaultKernelName();
        }

        Common::UniquePtr<IGeometryImporter> importer =
            GeometryKernelRegistry::instance().createImporter(kernelName, _options);
        if (!importer)
        {
            GeometryImportReport report;
            report.setFilePath(filePath);
            report.setKernelName(kernelName);
            report.setStatus(Common::Return::Failed);
            report.setMessage(Common::String("no geometry importer is available"));
            document.setLastReport(report);
            return report;
        }

        GeometryImportReport report = importer->importFile(filePath, document);
        report.setKernelName(importer->getName());
        document.setLastReport(report);
        return report;
    }

} // namespace EgLab::GE
