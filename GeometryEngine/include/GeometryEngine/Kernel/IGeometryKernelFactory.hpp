#pragma once

#include "Common/String.hpp"
#include "Common/UniquePtr.hpp"
#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Kernel/GeometryImportOptions.hpp"
#include "GeometryEngine/Kernel/GeometryKernelCapabilities.hpp"
#include "GeometryEngine/Kernel/IGeometryImporter.hpp"

namespace EgLab::GE
{
    /// Abstract factory for one geometry kernel (OCCT, Parasolid, ...).
    ///
    /// Adding a kernel means adding one factory implementation and registering it;
    /// nothing above this interface changes.
    class GeometryEngineAPI IGeometryKernelFactory
    {
    public:
        virtual ~IGeometryKernelFactory() = default;

        virtual Common::String getName() const = 0;

        virtual Common::String getVersion() const = 0;

        virtual GeometryKernelCapabilities getCapabilities() const = 0;

        virtual Common::UniquePtr<IGeometryImporter> createImporter(
            const GeometryImportOptions& options) const = 0;
    };

} // namespace EgLab::GE
