#pragma once

#include <cstdint>

#include "Common/String.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"

namespace EgLab::Platform
{
    /// Pick/selection filter bits for geometry models. One bit per topology type, using the
    /// GE::TopologyType numeric value as the bit index.
    constexpr uint32_t GEOMETRY_FEATURE_MASK_VERTEX = 1u << 0;
    constexpr uint32_t GEOMETRY_FEATURE_MASK_EDGE = 1u << 1;
    constexpr uint32_t GEOMETRY_FEATURE_MASK_FACE = 1u << 2;
    constexpr uint32_t GEOMETRY_FEATURE_MASK_SOLID = 1u << 3;
    constexpr uint32_t GEOMETRY_FEATURE_MASK_ALL = 0x0Fu;

    /// Stateless helpers describing a feature type filter mask.
    class GeometryFeatureMaskInfo
    {
    public:
        /// Returns the mask bit of a GE::TopologyType value.
        static uint32_t bitOf(uint32_t featureType);

        /// Reports whether a feature type passes the filter.
        static bool isEnabled(uint32_t mask, uint32_t featureType);

        /// Human readable list of the enabled feature types, e.g. "Face|Solid".
        static Common::String toString(uint32_t mask);
    };

} // namespace EgLab::Platform
