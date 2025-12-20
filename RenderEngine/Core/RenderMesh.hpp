#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Vector.hpp"
#include "Definites.hpp"

namespace EgLab
{

    class RenderMesh
    {
    public:
        RenderMesh()
        {
        }

        RenderMesh(DynamicArray<CoordType> vertices, DynamicArray<CoordType> normals,
                   DynamicArray<TexCoordType> texCoords)
        {
        }

    private:
        DynamicArray<CoordType> _vertices;
        DynamicArray<CoordType> _normals;
        DynamicArray<TexCoordType> _texCoords;
        DynamicArray<IdxType> _indices;
        // DynamicArray<Texture*> _textures;
    };

} // namespace EgLab