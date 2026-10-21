#pragma once
#include "MeshEngine/Common/Definitions.hpp"
#include "MeshEngine/Common/SoaContainer.hpp"

namespace EgLab::ME
{
    struct HENode
    {
        IdType _id;
        CoordType _coord;
        IdxType _halfEdge;
    };

    BEGIN_SOA_REG(HENode)
    SOA_REGISTERED_TYPE(HENode)
    REG_COL(IdType, _id)
    REG_COL(CoordType, _coord)
    REG_COL(IdxType, _halfEdge)
    END_SOA_REG()

    using HENodes = ME::SoaContainer<HENode>;

} // namespace EgLab::ME
