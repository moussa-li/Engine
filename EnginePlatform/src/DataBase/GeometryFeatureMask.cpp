#include "DataBase/GeometryFeatureMask.hpp"

namespace EgLab::Platform
{
    uint32_t GeometryFeatureMaskInfo::bitOf(uint32_t featureType)
    {
        if (featureType >= static_cast<uint32_t>(GE::TOPOLOGY_TYPE_COUNT))
        {
            return 0u;
        }
        return 1u << featureType;
    }

    bool GeometryFeatureMaskInfo::isEnabled(uint32_t mask, uint32_t featureType)
    {
        const uint32_t bit = bitOf(featureType);
        return bit != 0u && (mask & bit) != 0u;
    }

    Common::String GeometryFeatureMaskInfo::toString(uint32_t mask)
    {
        Common::String text;
        const struct
        {
            uint32_t bit;
            const char* name;
        } names[] = {
            {GEOMETRY_FEATURE_MASK_VERTEX, "Vertex"},
            {GEOMETRY_FEATURE_MASK_EDGE, "Edge"},
            {GEOMETRY_FEATURE_MASK_FACE, "Face"},
            {GEOMETRY_FEATURE_MASK_SOLID, "Solid"},
        };

        for (const auto& entry : names)
        {
            if ((mask & entry.bit) == 0u)
            {
                continue;
            }
            if (text.size() != 0)
            {
                text.append("|");
            }
            text.append(entry.name);
        }
        if (text.size() == 0)
        {
            return Common::String("None");
        }
        return text;
    }

} // namespace EgLab::Platform
