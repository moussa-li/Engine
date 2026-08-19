#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Vector.hpp"
#include "Core/Definites.hpp"
#include "Core/IndexBuffer.hpp"
#include "Core/VertexArray.hpp"

namespace EgLab::RE
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

        RenderMesh(Common::DynamicArray<CoordType> &vertices,
                   Common::DynamicArray<CoordType> &normals,
                   Common::DynamicArray<TexCoordType> &texCoords,
                   Common::DynamicArray<IdxType> &indices,
                   Common::DynamicArray<Texture *> &textures);

        ~RenderMesh();

        void setup();

        void draw(Common::SharedPtr<Shader> shader, Transform transform);

        void update();

        // void recalculateNormals();

    private:
        Common::DynamicArray<CoordType> _vertices;
        Common::DynamicArray<CoordType> _normals;
        Common::DynamicArray<TexCoordType> _texCoords;
        Common::DynamicArray<IdxType> _indices;
        Common::DynamicArray<Texture *> _textures;

        Common::SharedPtr<VertexArray> _vertexArray;
        Common::SharedPtr<VertexArray> _normalVertexArray;
        Common::SharedPtr<VertexArray> _textureVertexArray;
        Common::SharedPtr<VertexBuffer> _vertexBuffer;
        Common::SharedPtr<VertexBuffer> _normalVertexBuffer;
        Common::SharedPtr<VertexBuffer> _textureVertexBuffer;
        Common::SharedPtr<IndexBuffer> _indexBuffer;

        // static Shader* rec
    };

} // namespace EgLab::RE