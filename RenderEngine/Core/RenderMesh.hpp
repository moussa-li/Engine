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

    /// Render mesh storing vertex attributes, indices, and associated textures.
    class RenderMesh
    {
    public:
        /// @brief Creates an empty render mesh.
        RenderMesh();

        /**
         * @brief Creates a render mesh from vertex attributes and texture references.
         * @param vertices Mesh vertex positions.
         * @param normals Vertex normals.
         * @param texCoords Vertex texture coordinates.
         * @param indices Element indices.
         * @param textures Textures associated with the mesh.
         */
        RenderMesh(Common::DynamicArray<CoordType> &vertices,
                   Common::DynamicArray<CoordType> &normals,
                   Common::DynamicArray<TexCoordType> &texCoords,
                   Common::DynamicArray<IdxType> &indices,
                   Common::DynamicArray<Texture *> &textures);

        /// @brief Releases mesh rendering resources.
        ~RenderMesh();

        /// @brief Creates or updates GPU resources for this mesh.
        void setup();

        /**
         * @brief Draws the mesh using the specified shader and transform.
         * @param shader Shader used for rendering.
         * @param transform Model transform applied to the mesh.
         */
        void draw(Common::SharedPtr<Shader> shader, Transform transform);

        /// @brief Updates mesh rendering data.
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