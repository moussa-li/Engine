#pragma once

#include <cstdint>

namespace EgLab::Platform
{
    struct MeshPODWireHeader
    {
        uint32_t nodeNumber{0};
        uint32_t elemNumber{0};
        uint32_t nodeIdsOffset{0};
        uint32_t nodeDataOffset{0};
        uint32_t elemIdsOffset{0};
        uint32_t elemTypesOffset{0};
        uint32_t elemDataOffset{0};
        uint32_t totalDataSize{0};
    };
} // namespace EgLab::Platform