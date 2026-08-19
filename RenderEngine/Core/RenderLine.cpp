#include "RenderEngine/Core/RenderLine.hpp"

#include <GL/glew.h>

#include "Common/Utils.hpp"
#include "RenderEngine/Core/Shader.hpp"
#include "RenderEngine/Core/VertexBufferLayout.hpp"

namespace EgLab::RE
{
    RenderLine::~RenderLine()
    {
    }
    void RenderLine::setNodes(Common::DynamicArray<CoordType>&& nodes)
    {
        _vertices = Common::move(nodes);
    }

    void RenderLine::setIndices(Common::DynamicArray<IdxType>&& idxs)
    {
        _indices = Common::move(idxs);
    }

    void RenderLine::setup()
    {
        _vertexArray = Common::makeShared<VertexArray>();
        _vertexArray->bind();
        VertexBufferLayout layout;
        _vertexBuffer = Common::makeShared<VertexBuffer>(_vertices);
        layout.pushBack<float>(3, _vertexBuffer);

        _vertexArray->addBuffer(layout);
        _indexBuffer = Common::makeShared<IndexBuffer>(_indices);

        _vertexArray->unBind();
    }

    void RenderLine::draw(Common::SharedPtr<Shader> shader)
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

} // namespace EgLab::RE