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

    void RenderLine::drawHighlight(Common::SharedPtr<Shader> shader, IdxType lineIndex)
    {
        const size_t indexOffset = static_cast<size_t>(lineIndex) * 2;
        if (indexOffset + 1 >= _indices.size())
        {
            return;
        }

        GLint previousDepthFunction = GL_LESS;
        GLboolean previousDepthMask = GL_TRUE;
        GLfloat previousLineWidth = 1.0f;
        glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunction);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask);
        glGetFloatv(GL_LINE_WIDTH, &previousLineWidth);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        shader->bind();
        shader->setUniform4f("u_Color", Common::Vector4f(1.0f, 0.28f, 0.04f, 1.0f));
        _vertexArray->bind();
        _indexBuffer->bind();
        glLineWidth(3.0f);
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(indexOffset * sizeof(IdxType)));
        _vertexArray->unBind();
        _indexBuffer->unBind();
        shader->unBind();

        glLineWidth(previousLineWidth);
        glDepthMask(previousDepthMask);
        glDepthFunc(static_cast<GLenum>(previousDepthFunction));
    }

    IdxType RenderLine::touch(const PickContext& context)
    {
        if (context.width == 0 || context.height == 0 || context.radius < 0.0f)
        {
            return static_cast<IdxType>(-1);
        }

        const Scalar radiusSquared = context.radius * context.radius;
        Scalar closestDistanceSquared = std::numeric_limits<Scalar>::max();
        Scalar closestDepth = std::numeric_limits<Scalar>::max();
        IdxType touchedIndex = static_cast<IdxType>(-1);

        for (size_t i = 0; i + 1 < _indices.size(); i += 2)
        {
            const IdxType index0 = _indices[i];
            const IdxType index1 = _indices[i + 1];
            if (index0 >= _vertices.size() || index1 >= _vertices.size())
            {
                continue;
            }

            PickUtils::ScreenPoint point0;
            PickUtils::ScreenPoint point1;
            if (!PickUtils::projectPoint(_vertices[index0], context, point0) ||
                !PickUtils::projectPoint(_vertices[index1], context, point1))
            {
                continue;
            }

            const Scalar segmentX = point1.x - point0.x;
            const Scalar segmentY = point1.y - point0.y;
            const Scalar segmentLengthSquared = segmentX * segmentX + segmentY * segmentY;
            Scalar t = 0.0f;
            if (segmentLengthSquared > 1e-12f)
            {
                t = ((context.x - point0.x) * segmentX + (context.y - point0.y) * segmentY) /
                    segmentLengthSquared;
                t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
            }

            const Scalar deltaX = context.x - (point0.x + t * segmentX);
            const Scalar deltaY = context.y - (point0.y + t * segmentY);
            const Scalar distanceSquared = deltaX * deltaX + deltaY * deltaY;
            const Scalar depth = point0.depth + t * (point1.depth - point0.depth);
            if (distanceSquared <= radiusSquared &&
                (depth < closestDepth - 1e-6f || (std::abs(depth - closestDepth) <= 1e-6f &&
                                                  distanceSquared < closestDistanceSquared)))
            {
                touchedIndex = static_cast<IdxType>(i / 2);
                closestDistanceSquared = distanceSquared;
                closestDepth = depth;
            }
        }

        if (touchedIndex == static_cast<IdxType>(-1) || closestDepth > context.bestDepth + 1e-6f ||
            (std::abs(closestDepth - context.bestDepth) <= 1e-6f &&
             closestDistanceSquared >= context.bestDistanceSquared))
        {
            return static_cast<IdxType>(-1);
        }

        context.bestDepth = closestDepth;
        context.bestDistanceSquared = closestDistanceSquared;
        return touchedIndex;
    }

} // namespace EgLab::RE