#include "RenderMesh.hpp"

#include <GL/glew.h>

#include "Common/HashSet.hpp"
#include "GLWrapper.hpp"
#include "IndexBuffer.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Transform.hpp"
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "VertexBufferLayout.hpp"

namespace EgLab
{
    RenderMesh::RenderMesh()
    {
    }

    RenderMesh::RenderMesh(DynamicArray<CoordType> &vertices, DynamicArray<CoordType> &normals,
                           DynamicArray<TexCoordType> &texCoords, DynamicArray<IdxType> &indices,
                           DynamicArray<Texture *> &textures)
    {
        _vertices = vertices;
        _normals = normals;
        _texCoords = texCoords;
        _indices = indices;
        _textures = textures;
        setup();
    }

    void RenderMesh::setup()
    {
        _vertexArray = makeShared<VertexArray>();

        VertexBufferLayout layout;
        _vertexBuffer = makeShared<VertexBuffer>(_vertices);
        layout.pushBack<float>(3, _vertexBuffer);

        _normalVertexBuffer = makeShared<VertexBuffer>(_normals);
        layout.pushBack<float>(3, _normalVertexBuffer);

        _textureVertexBuffer = makeShared<VertexBuffer>(_texCoords);
        layout.pushBack<float>(2, _textureVertexBuffer);

        _vertexArray->addBuffer(layout);
        _indexBuffer = makeShared<IndexBuffer>(_indices);
        _vertexArray->unBind();
    }

    void RenderMesh::draw(SharedPtr<Shader> shader, Transform transform)
    {
        shader->bind();
        shader->setUniformMat4f("model", transform.getMatrix());
        // shader->setUniform1i();
        _vertexArray->bind();
        _indexBuffer->bind();
        glDrawElements(GL_TRIANGLES, _indexBuffer->getCount(), GL_UNSIGNED_INT, 0);
        GL_CHECK();
        glDepthMask(GL_TRUE);
        _vertexArray->unBind();
        _indexBuffer->unBind();
        shader->unBind();
    }

    void RenderMesh::update()
    {
    }

} // namespace EgLab