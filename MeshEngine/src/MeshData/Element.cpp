#include "MeshEngine/MeshData/Element.hpp"

namespace EgLab::ME
{

    int getTypeDim(ElemType type)
    {
        return (int)type >> 2;
    }

} // namespace EgLab::ME
