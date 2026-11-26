#pragma once

#include "Common/Singleton.hpp"
#include "GeometryEngine/Kernel/GeometryImportOptions.hpp"

namespace EgLab::Platform
{
    /// Global tessellation parameters used when a B-Rep file is imported (and re-imported).
    ///
    /// The geometry render settings panel and the SetGeometryTessellation command update these
    /// values; ImportGeometry reads them so the UI/AI can steer the render mesh density without
    /// binding the platform layer to a specific kernel adapter.
    class GeometryTessellationSettings : public Common::Singleton<GeometryTessellationSettings>
    {
    public:
        /// Returns a copy of the current import/tessellation options.
        GE::GeometryImportOptions getOptions() const
        {
            return _options;
        }

        /// Replaces the tessellation options.
        void setOptions(const GE::GeometryImportOptions& options)
        {
            _options = options;
        }

    private:
        friend class Common::Singleton<GeometryTessellationSettings>;

        GeometryTessellationSettings() = default;
        ~GeometryTessellationSettings() = default;

        GE::GeometryImportOptions _options;
    };
} // namespace EgLab::Platform
