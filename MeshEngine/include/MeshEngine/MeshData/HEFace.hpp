#pragma once
#include "MeshEngine/Common/Definitions.hpp"
#include "MeshEngine/Common/SoaContainer.hpp"

namespace EgLab::ME
{
    struct HEFace
    {
        IdType _id;
        IdxType _halfEdge;
    };

    BEGIN_SOA_REG(HEFace)
    SOA_REGISTERED_TYPE(HEFace)
    REG_COL(IdType, _id)
    REG_COL(IdxType, _halfEdge)
    END_SOA_REG()

    using HEFaces = ME::SoaContainer<HEFace>;
} // namespace EgLab::ME
