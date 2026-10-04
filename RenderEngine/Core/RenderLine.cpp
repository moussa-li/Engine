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

    void RenderLine::setColor(const Common::Vector4f& color)
    {
        _color = color;
        _hasColor = true;
    }

    Common::BBox<Scalar, 3> RenderLine::getBounds() const
    {
        Common::BBox<Scalar, 3> bounds;
        for (auto it = _vertices.begin(); it.hasNext(); it.next())
        {
            bounds.addPoint(it.data());
        }
        return bounds;
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
        if (_hasColor)
        {
            shader->setUniform4f("u_Color", _color);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonOffset(-1.0f, -1.0f);
        }
        _vertexArray->bind();
        _indexBuffer->bind();
        glLineWidth(_hasColor ? 2.0f : 1.0f);
        glDrawElements(GL_LINES, _indices.size(), GL_UNSIGNED_INT, 0);
        if (_hasColor)
        {
            glLineWidth(1.0f);
            glDisable(GL_POLYGON_OFFSET_LINE);
        }
        glDepthMask(GL_TRUE);
        _vertexArray->unBind();
        _indexBuffer->unBind();
        shader->unBind();
    }

} // namespace EgLab::RE