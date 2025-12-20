#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Vector.hpp"

namespace EgLab
{
    class RenderMesh
    {
    private:
        DynamicArray<Vector4f> _vertices;
        DynamicArray<Vector4f> _normals;
        DynamicArray<Vector4f> _texCoords;
        DynamicArray<size_t> _indices;
        // DynamicArray<Texture*> _textures;
    };

} // namespace EgLab