#pragma once

#include "GeometryEngine/Kernel/IGeometryKernelFactory.hpp"

namespace EgLab::GE
{
    /// Fallback adapter used when no third-party kernel is compiled in.
    class NullGeometryKernelFactory : public IGeometryKernelFactory
    {
    public:
        Common::String getName() const override;

        Common::String getVersion() const override;

        GeometryKernelCapabilities getCapabilities() const override;

        Common::UniquePtr<IGeometryImporter> createImporter(
            const GeometryImportOptions& options) const override;
    };

} // namespace EgLab::GE
