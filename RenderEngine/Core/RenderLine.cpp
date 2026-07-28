#include "RenderEngine/Core/RenderLine.hpp"

#include <GL/glew.h>

#include "Common/Utils.hpp"
#include "RenderEngine/Core/Shader.hpp"
#include "RenderEngine/Core/VertexBufferLayout.hpp"

namespace EgLab
{
    void RenderLine::setNodes(DynamicArray<CoordType>&& nodes)
    {
        _vertices = nodes;
    }

    void RenderLine::setIndices(DynamicArray<IdxType>&& idxs)
    {
        _indices = idxs;
    }

    void RenderLine::setup()
    {
        _vertexArray = makeShared<VertexArray>();
        _vertexArray->bind();
        VertexBufferLayout layout;
        _vertexBuffer = makeShared<VertexBuffer>(_vertices);
        layout.pushBack<float>(3, _vertexBuffer);

        _vertexArray->addBuffer(layout);
        _indexBuffer = makeShared<IndexBuffer>(_indices);

        _vertexArray->unBind();
    }

    void RenderLine::draw(SharedPtr<Shader> shader)
    {
        shader->bind();
        _vertexArray->bind();
        _indexBuffer->bind();
        glLineWidth(1.0f);
        glDrawElements(GL_LINES, _indices.size(), GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);
        _vertexArray->unBind();
        _indexBuffer->unBind();
        shader->unBind();
    }

} // namespace EgLab