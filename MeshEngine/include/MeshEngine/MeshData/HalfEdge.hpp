#pragma once

#include "MeshEngine/Common/Definitions.hpp"
#include "MeshEngine/Common/SoaContainer.hpp"

namespace EgLab::ME
{
    struct HalfEdge
    {
        IdxType face;
        IdxType vertex;
        IdxType next;
        IdxType prev;
        IdxType twin;
    };

    BEGIN_SOA_REG(HalfEdge)
    SOA_REGISTERED_TYPE(HalfEdge)
    REG_COL(IdxType, face)
    REG_COL(IdxType, vertex)
    REG_COL(IdxType, next)
    REG_COL(IdxType, prev)
    REG_COL(IdxType, twin)
    END_SOA_REG()

    using HalfEdges = ME::SoaContainer<HalfEdge>;

} // namespace EgLab::ME