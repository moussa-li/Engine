#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Singleton.hpp"
#include "Common/String.hpp"
#include "Common/UniquePtr.hpp"
#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Kernel/IGeometryKernelFactory.hpp"

namespace EgLab::GE
{
    constexpr const char* DEFAULT_GEOMETRY_KERNEL = "OCCT";

    constexpr const char* NULL_GEOMETRY_KERNEL = "Null";

    /// Process-wide registry of geometry kernel factories.
    ///
    /// The Null adapter is always registered, so createImporter() never returns null.
    class GeometryEngineAPI GeometryKernelRegistry : public Common::Singleton<GeometryKernelRegistry>
    {
    public:
        bool registerFactory(Common::UniquePtr<IGeometryKernelFactory> factory);

        const IGeometryKernelFactory* findFactory(const Common::String& name) const;

        /// Creates an importer for the requested kernel. Unknown or unavailable names fall
        /// back to the Null adapter so callers can report a diagnosable error.
        Common::UniquePtr<IGeometryImporter> createImporter(const Common::String& name,
                                                            const GeometryImportOptions& options) const;

        Common::DynamicArray<Common::String> getKernelNames() const;

        /// Kernel selected when the caller does not specify one.
        static Common::String getDefaultKernelName();

    private:
        GeometryKernelRegistry();

        ~GeometryKernelRegistry();

        friend class Common::Singleton<GeometryKernelRegistry>;

        class Impl;
        Common::UniquePtr<Impl> _impl;
    };

} // namespace EgLab::GE
