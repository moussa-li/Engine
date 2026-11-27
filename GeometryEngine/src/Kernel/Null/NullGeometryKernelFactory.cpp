#include "NullGeometryKernelFactory.hpp"

#include "GeometryEngine/Kernel/GeometryKernelRegistry.hpp"

namespace EgLab::GE
{
    namespace
    {
        class NullGeometryImporter : public IGeometryImporter
        {
        public:
            Common::String getName() const override
            {
                return Common::String(NULL_GEOMETRY_KERNEL);
            }

            GeometryImportReport importFile(const Common::String& filePath,
                                            GeometryDocument& document) const override
            {
                GeometryImportReport report;
                report.setFilePath(filePath);
                report.setKernelName(getName());
                report.setStatus(Common::Return::NotImplacementYet);
                report.setMessage(Common::String(
                    "GeometryEngine was built without a third-party kernel adapter "
                    "(configure with -DBUILD_GEOMETRY_KERNEL=OCCT to enable import)."));
                return report;
            }

            GeometryImportReport importBuffer(const void* data, std::size_t size,
                                              GeometryDocument& document) const override
            {
                (void)data;
                (void)size;
                (void)document;
                GeometryImportReport report;
                report.setKernelName(getName());
                report.setStatus(Common::Return::NotImplacementYet);
                report.setMessage(Common::String(
                    "GeometryEngine was built without a third-party kernel adapter "
                    "(configure with -DBUILD_GEOMETRY_KERNEL=OCCT to enable import)."));
                return report;
            }
        };
    } // namespace

    Common::String NullGeometryKernelFactory::getName() const
    {
        return Common::String(NULL_GEOMETRY_KERNEL);
    }

    Common::String NullGeometryKernelFactory::getVersion() const
    {
        return Common::String("0.0.0");
    }

    GeometryKernelCapabilities NullGeometryKernelFactory::getCapabilities() const
    {
        return GeometryKernelCapabilities();
    }

    Common::UniquePtr<IGeometryImporter> NullGeometryKernelFactory::createImporter(
        const GeometryImportOptions& options) const
    {
        (void)options;
        return Common::UniquePtr<IGeometryImporter>(new NullGeometryImporter());
    }

} // namespace EgLab::GE
