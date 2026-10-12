#include "RenderEngine/Core/RenderNode.hpp"

#include <GL/glew.h>

#include "Common/Utils.hpp"
#include "RenderEngine/Core/Shader.hpp"
#include "RenderEngine/Core/VertexArray.hpp"
#include "RenderEngine/Core/VertexBufferLayout.hpp"

namespace EgLab::RE
{
    RenderNode::~RenderNode()
    {
    }

    void RenderNode::setNodes(Common::DynamicArray<CoordType> &&nodes)
    {
        _vertices = nodes;
    }

    Common::BBox<Scalar, 3> RenderNode::getBounds() const
    {
        Common::BBox<Scalar, 3> bounds;
        for (auto it = _vertices.begin(); it.hasNext(); it.next())
        {
            bounds.addPoint(it.data());
        }
        return bounds;
    }

    void RenderNode::setup()
    {
        _vertexArray = Common::makeShared<VertexArray>();
        _vertexArray->bind();
        VertexBufferLayout layout;
        _vertexBuffer = Common::makeShared<VertexBuffer>(_vertices);
        layout.pushBack<float>(3, _vertexBuffer);

        _vertexArray->addBuffer(layout);
        _vertexArray->unBind();
    }

    void RenderNode::draw(Common::SharedPtr<Shader> shader)
    {
        shader->bind();
        shader->setUniform1f("u_PointSize", 3.0f);
        shader->setUniform4f("u_Color", Common::Vector4f(0.18f, 0.24f, 0.30f, 1.0f));
        _vertexArray->bind();
        glDrawArrays(GL_POINTS, 0, _vertices.size());
        glDepthMask(GL_TRUE);
        glEnable(GL_PROGRAM_POINT_SIZE);
        _vertexArray->unBind();
        shader->unBind();
    }

    void RenderNode::drawHighlight(Common::SharedPtr<Shader> shader, IdxType nodeIndex)
    {
        if (nodeIndex >= _vertices.size())
        {
            return;
        }

        GLint previousDepthFunction = GL_LESS;
        glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunction);
        glDepthFunc(GL_LEQUAL);

        shader->bind();
        shader->setUniform1f("u_PointSize", 10.0f);
        shader->setUniform4f("u_Color", Common::Vector4f(1.0f, 0.28f, 0.04f, 1.0f));
        _vertexArray->bind();
        glDrawArrays(GL_POINTS, static_cast<GLint>(nodeIndex), 1);
        _vertexArray->unBind();
        shader->unBind();

        glDepthFunc(static_cast<GLenum>(previousDepthFunction));
    }

    IdxType RenderNode::touch(const PickContext &context)
    {
        if (context.width == 0 || context.height == 0 || context.radius < 0.0f)
        {
            return static_cast<IdxType>(-1);
        }

        const Scalar radiusSquared = context.radius * context.radius;
        Scalar closestDistanceSquared = std::numeric_limits<Scalar>::max();
        Scalar closestDepth = std::numeric_limits<Scalar>::max();
        IdxType touchedIndex = static_cast<IdxType>(-1);

        for (IdxType i = 0; i < _vertices.size(); ++i)
        {
            const auto &vertex = _vertices[i];
            const Scalar eyeX = context.view[0][0] * vertex.x() + context.view[0][1] * vertex.y() +
                                context.view[0][2] * vertex.z() + context.view[0][3];
            const Scalar eyeY = context.view[1][0] * vertex.x() + context.view[1][1] * vertex.y() +
                                context.view[1][2] * vertex.z() + context.view[1][3];
            const Scalar eyeZ = context.view[2][0] * vertex.x() + context.view[2][1] * vertex.y() +
                                context.view[2][2] * vertex.z() + context.view[2][3];
            const Scalar eyeW = context.view[3][0] * vertex.x() + context.view[3][1] * vertex.y() +
                                context.view[3][2] * vertex.z() + context.view[3][3];
            const Scalar clipX = context.projection[0][0] * eyeX + context.projection[0][1] * eyeY +
                                 context.projection[0][2] * eyeZ + context.projection[0][3] * eyeW;
            const Scalar clipY = context.projection[1][0] * eyeX + context.projection[1][1] * eyeY +
                                 context.projection[1][2] * eyeZ + context.projection[1][3] * eyeW;
            const Scalar clipZ = context.projection[2][0] * eyeX + context.projection[2][1] * eyeY +
                                 context.projection[2][2] * eyeZ + context.projection[2][3] * eyeW;
            const Scalar clipW = context.projection[3][0] * eyeX + context.projection[3][1] * eyeY +
                                 context.projection[3][2] * eyeZ + context.projection[3][3] * eyeW;
            if (clipW <= 1e-6f)
            {
                continue;
            }

            const Scalar ndcX = clipX / clipW;
            const Scalar ndcY = clipY / clipW;
            const Scalar ndcZ = clipZ / clipW;
            if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f || ndcZ < -1.0f ||
                ndcZ > 1.0f)
            {
                continue;
            }

            const Scalar screenX = (ndcX + 1.0f) * 0.5f * context.width;
            const Scalar screenY = (1.0f - ndcY) * 0.5f * context.height;
            const Scalar deltaX = screenX - context.x;
            const Scalar deltaY = screenY - context.y;
            const Scalar distanceSquared = deltaX * deltaX + deltaY * deltaY;
            if (distanceSquared <= radiusSquared &&
                (ndcZ < closestDepth - 1e-6f || (std::abs(ndcZ - closestDepth) <= 1e-6f &&
                                                 distanceSquared < closestDistanceSquared)))
            {
                touchedIndex = i;
                closestDistanceSquared = distanceSquared;
                closestDepth = ndcZ;
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
