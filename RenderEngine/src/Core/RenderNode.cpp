#include "RenderEngine/Core/RenderNode.hpp"

#include <GL/glew.h>

#include "Common/Utils.hpp"
#include "RenderEngine/Core/IndexBuffer.hpp"
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
        _vertices = Common::move(nodes);
        rebuildHiddenFlags();
        // Cached highlight indices refer to the previous vertex layout.
        _highlightIndexBuffer = nullptr;
        _highlightIndices.clear();
    }

    void RenderNode::setHiddenNodes(const Common::HashSet<IdxType> &hiddenNodeIndices)
    {
        _hiddenNodeIndices = hiddenNodeIndices;
        rebuildHiddenFlags();
        _hiddenFlagsDirty = true;
    }

    void RenderNode::rebuildHiddenFlags()
    {
        _hiddenFlags.resize(_vertices.size());
        for (size_t index = 0; index < _hiddenFlags.size(); ++index)
        {
            _hiddenFlags[index] = 0.0f;
        }

        if (_hiddenNodeIndices.empty())
        {
            return;
        }

        for (auto it = _hiddenNodeIndices.begin(); it.hasNext(); it.next())
        {
            const IdxType nodeIndex = it.data();
            if (nodeIndex < _hiddenFlags.size())
            {
                _hiddenFlags[nodeIndex] = 1.0f;
            }
        }
    }

    void RenderNode::uploadHiddenFlagsIfNeeded()
    {
        if (!_hiddenFlagsDirty || !_hiddenVertexBuffer || _hiddenFlags.empty())
        {
            _hiddenFlagsDirty = false;
            return;
        }

        _hiddenVertexBuffer->setData(_hiddenFlags.data(), _hiddenFlags.size() * sizeof(float));
        _hiddenFlagsDirty = false;
    }

    void RenderNode::setColor(const Common::Vector4f &color)
    {
        _color = color;
    }

    void RenderNode::setPointSize(float pointSize)
    {
        _pointSize = pointSize;
    }

    void RenderNode::setDepthTestEnabled(bool enabled)
    {
        _depthTestEnabled = enabled;
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
        rebuildHiddenFlags();
        _vertexArray = Common::makeShared<VertexArray>();
        _vertexArray->bind();
        VertexBufferLayout layout;
        _vertexBuffer = Common::makeShared<VertexBuffer>(_vertices);
        layout.pushBack<float>(3, _vertexBuffer);
        if (!_hiddenFlags.empty())
        {
            _hiddenVertexBuffer =
                Common::makeShared<VertexBuffer>(_hiddenFlags.data(),
                                                 _hiddenFlags.size() * sizeof(float));
            layout.pushBack<float>(1, _hiddenVertexBuffer);
        }

        _vertexArray->addBuffer(layout);
        _vertexArray->unBind();
        _hiddenFlagsDirty = false;
    }

    void RenderNode::draw(Common::SharedPtr<Shader> shader)
    {
        const GLboolean depthTestWasEnabled = glIsEnabled(GL_DEPTH_TEST);
        if (!_depthTestEnabled)
        {
            glDisable(GL_DEPTH_TEST);
        }
        glEnable(GL_PROGRAM_POINT_SIZE);
        uploadHiddenFlagsIfNeeded();
        shader->bind();
        shader->setUniform1f("u_PointSize", _pointSize);
        shader->setUniform4f("u_Color", _color);
        _vertexArray->bind();
        glDrawArrays(GL_POINTS, 0, _vertices.size());
        glDepthMask(GL_TRUE);
        _vertexArray->unBind();
        shader->unBind();
        if (depthTestWasEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
    }

    void RenderNode::drawHighlight(Common::SharedPtr<Shader> shader, IdxType nodeIndex)
    {
        if (nodeIndex >= _vertices.size())
        {
            return;
        }
        if (!_hiddenNodeIndices.empty() &&
            _hiddenNodeIndices.find(nodeIndex) != _hiddenNodeIndices.end())
        {
            return;
        }

        GLint previousDepthFunction = GL_LESS;
        GLfloat previousDepthRange[2] = {0.0f, 1.0f};
        glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunction);
        glGetFloatv(GL_DEPTH_RANGE, previousDepthRange);
        glDepthFunc(GL_LEQUAL);
        // Points ignore glPolygonOffset, so squeeze the near depth range: the highlighted
        // vertex stays visible on top of the surface it lies on.
        glDepthRange(0.0, 0.999f);

        shader->bind();
        shader->setUniform1f("u_PointSize", 10.0f);
        shader->setUniform4f("u_Color", Common::Vector4f(1.0f, 0.28f, 0.04f, 1.0f));
        _vertexArray->bind();
        glDrawArrays(GL_POINTS, static_cast<GLint>(nodeIndex), 1);
        _vertexArray->unBind();
        shader->unBind();

        glDepthRange(previousDepthRange[0], previousDepthRange[1]);
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
            if (!_hiddenNodeIndices.empty() &&
                _hiddenNodeIndices.find(i) != _hiddenNodeIndices.end())
            {
                continue;
            }

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

        if (touchedIndex == static_cast<IdxType>(-1) ||
            !context.acceptHit(PickKind::Node, closestDepth, closestDistanceSquared))
        {
            return static_cast<IdxType>(-1);
        }

        return touchedIndex;
    }

    void RenderNode::touchRect(const RectPickContext &context,
                               Common::DynamicArray<IdxType> &outIndices) const
    {
        if (context.width == 0 || context.height == 0)
        {
            return;
        }

        context.prepare();

        const size_t vertexCount = _vertices.size();
        for (IdxType index = 0; index < vertexCount; ++index)
        {
            if (!_hiddenNodeIndices.empty() &&
                _hiddenNodeIndices.find(index) != _hiddenNodeIndices.end())
            {
                continue;
            }

            PickUtils::ScreenPoint point;
            if (!context.project(_vertices[index], point))
            {
                continue;
            }
            // A point is fully inside the rectangle exactly when it intersects it.
            if (context.contains(point.x, point.y))
            {
                outIndices.pushBack(index);
            }
        }
    }

    void RenderNode::drawHighlightBatch(Common::SharedPtr<Shader> shader,
                                        const Common::DynamicArray<IdxType> &indices)
    {
        if (indices.empty() || _vertexArray == nullptr)
        {
            return;
        }

        // Sparse node selections rarely form contiguous runs, so they are drawn from a reused
        // index buffer: one draw call regardless of how scattered the selection is.
        Common::DynamicArray<IdxType> validIndices;
        validIndices.reserve(indices.size());
        for (size_t index = 0; index < indices.size(); ++index)
        {
            if (indices[index] >= _vertices.size())
            {
                continue;
            }
            if (!_hiddenNodeIndices.empty() &&
                _hiddenNodeIndices.find(indices[index]) != _hiddenNodeIndices.end())
            {
                continue;
            }
            validIndices.pushBack(indices[index]);
        }
        if (validIndices.empty())
        {
            _highlightIndexBuffer = nullptr;
            _highlightIndices.clear();
            return;
        }

        bool indicesChanged = validIndices.size() != _highlightIndices.size();
        for (size_t index = 0; !indicesChanged && index < validIndices.size(); ++index)
        {
            indicesChanged = validIndices[index] != _highlightIndices[index];
        }
        if (indicesChanged)
        {
            _highlightIndexBuffer = Common::makeShared<IndexBuffer>(validIndices);
            _highlightIndices = Common::move(validIndices);
        }

        GLint previousDepthFunction = GL_LESS;
        glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunction);
        glDepthFunc(GL_LEQUAL);

        shader->bind();
        shader->setUniform1f("u_PointSize", 10.0f);
        shader->setUniform4f("u_Color", Common::Vector4f(1.0f, 0.28f, 0.04f, 1.0f));
        _vertexArray->bind();
        _highlightIndexBuffer->bind();
        glDrawElements(GL_POINTS, static_cast<GLsizei>(_highlightIndexBuffer->getCount()),
                       GL_UNSIGNED_INT, 0);
        _vertexArray->unBind();
        _highlightIndexBuffer->unBind();
        shader->unBind();

        glDepthFunc(static_cast<GLenum>(previousDepthFunction));
    }
} // namespace EgLab::RE
