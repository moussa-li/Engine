#include "OcctGeometryKernelFactory.hpp"

#include <Standard_Version.hxx>

#include "Common/StringTools.hpp"
#include "OcctGeometryImporter.hpp"

namespace EgLab::GE
{
    Common::String OcctGeometryKernelFactory::getName() const
    {
        return Common::String("OCCT");
    }

    Common::String OcctGeometryKernelFactory::getVersion() const
    {
        Common::String version;
        version.append(Common::ToString(static_cast<int>(OCC_VERSION_MAJOR)).c_str());
        version.append(".");
        version.append(Common::ToString(static_cast<int>(OCC_VERSION_MINOR)).c_str());
        version.append(".");
        version.append(Common::ToString(static_cast<int>(OCC_VERSION_MAINTENANCE)).c_str());
        return version;
    }

    GeometryKernelCapabilities OcctGeometryKernelFactory::getCapabilities() const
    {
        GeometryKernelCapabilities capabilities;
        capabilities.add(GeometryKernelCapability::StepImport);
        capabilities.add(GeometryKernelCapability::BrepImport);
        capabilities.add(GeometryKernelCapability::Tessellation);
        return capabilities;
    }

    Common::UniquePtr<IGeometryImporter> OcctGeometryKernelFactory::createImporter(
        const GeometryImportOptions& options) const
    {
        return Common::UniquePtr<IGeometryImporter>(new OcctGeometryImporter(options));
    }

} // namespace EgLab::GE
