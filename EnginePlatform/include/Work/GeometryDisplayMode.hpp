#pragma once

#include <cstdint>

#include "Common/String.hpp"

namespace EgLab::Platform
{
    /// How imported B-Rep geometry models are drawn.
    enum class GeometryDisplayMode : uint32_t
    {
        /// Shaded faces plus the dense triangle wireframe (the regular mesh appearance).
        Mesh = 0,
        /// Shaded faces plus only each face's outer frame (no interior triangle lines).
        FaceOutline = 1,
        /// No shaded faces, only the geometry outline lines.
        EdgeOnly = 2,
    };

    /// Stateless descriptions and validation for GeometryDisplayMode.
    class GeometryDisplayModeInfo
    {
    public:
        static constexpr uint32_t Count = 3;

        static bool isValid(uint32_t value)
        {
            return value < Count;
        }

        static const char* toString(GeometryDisplayMode mode)
        {
            switch (mode)
            {
            case GeometryDisplayMode::Mesh:
                return "Mesh";
            case GeometryDisplayMode::FaceOutline:
                return "FaceOutline";
            case GeometryDisplayMode::EdgeOnly:
                return "EdgeOnly";
            }
            return "Unknown";
        }

        static Common::String toDisplayName(GeometryDisplayMode mode)
        {
            switch (mode)
            {
            case GeometryDisplayMode::Mesh:
                return Common::String("Mesh (faces + triangle wireframe)");
            case GeometryDisplayMode::FaceOutline:
                return Common::String("Face outline (faces + face frames)");
            case GeometryDisplayMode::EdgeOnly:
                return Common::String("Edges only (no faces)");
            }
            return Common::String("Unknown");
        }
    };

} // namespace EgLab::Platform
