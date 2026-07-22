#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Vector.hpp"
#include "Core/Definites.hpp"
#include "Core/IndexBuffer.hpp"
#include "Core/VertexArray.hpp"

namespace EgLab
{

    class Shader;
    class Transform;
    class Texture;
    class VertexArray;
    class VertexBuffer;
    class IndexBuffer;
    class ShaderStorageBuffer;

    class RenderMesh
    {
    public:
        RenderMesh();

        RenderMesh(DynamicArray<CoordType> &vertices, DynamicArray<CoordType> &normals,
                   DynamicArray<TexCoordType> &texCoords, DynamicArray<IdxType> &indices,
                   DynamicArray<Texture *> &textures);

        ~RenderMesh()
        {
        }

        void setup();

        void draw(SharedPtr<Shader> shader, Transform transform);

        void update();

        // void recalculateNormals();

    private:
        DynamicArray<CoordType> _vertices;
        DynamicArray<CoordType> _normals;
        DynamicArray<TexCoordType> _texCoords;
        DynamicArray<IdxType> _indices;
        DynamicArray<Texture *> _textures;

        SharedPtr<VertexArray> _vertexArray;
        SharedPtr<VertexArray> _normalVertexArray;
        SharedPtr<VertexArray> _textureVertexArray;
        SharedPtr<VertexBuffer> _vertexBuffer;
        SharedPtr<VertexBuffer> _normalVertexBuffer;
        SharedPtr<VertexBuffer> _textureVertexBuffer;
        SharedPtr<IndexBuffer> _indexBuffer;

        // static Shader* rec
    };

} // namespace EgLab