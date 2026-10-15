#include "RenderEngine/Core/RenderFace.hpp"

#include <GL/glew.h>

#include "Common/Performance.hpp"
#include "Common/Utils.hpp"
#include "RenderEngine/Core/Shader.hpp"
#include "RenderEngine/Core/VertexBufferLayout.hpp"

namespace EgLab::RE
{
    RenderFace::~RenderFace()
    {
        if (_colorSSBO != 0)
        {
            glDeleteBuffers(1, &_colorSSBO);
            _colorSSBO = 0;
        }
    }

    void RenderFace::setup()
    {
        if (_normals.size() != _vertices.size())
        {
            _normals.clear();
            _normals.resize(_vertices.size());

            Common::Performance perf;
            perf.start();
            for (size_t i = 0; i + 2 < _indices.size(); i += 3)
            {
                const IdxType i0 = _indices[i];
                const IdxType i1 = _indices[i + 1];
                const IdxType i2 = _indices[i + 2];
                if (i0 >= _vertices.size() || i1 >= _vertices.size() || i2 >= _vertices.size())
                {
                    continue;
                }

                CoordType normal =
                    (_vertices[i1] - _vertices[i0]).cross(_vertices[i2] - _vertices[i0]);
                _normals[i0] = _normals[i0] + normal;
                _normals[i1] = _normals[i1] + normal;
                _normals[i2] = _normals[i2] + normal;
            }

            for (size_t i = 0; i < _normals.size(); ++i)
            {
                _normals[i].normalize();
            }

            perf.stop();

            LOG(INFO) << "Finished calculating normals for RenderFace: "
                      << ", end_ms=" << perf.getEndTimestampMs()
                      << ", elapsed_ms=" << perf.getElapsedMilliseconds();
        }

        _vertexArray = Common::makeShared<VertexArray>();
        _vertexArray->bind();

        VertexBufferLayout layout;
        _vertexBuffer = Common::makeShared<VertexBuffer>(_vertices);
        layout.pushBack<float>(3, _vertexBuffer);
        _normalVertexBuffer = Common::makeShared<VertexBuffer>(_normals);
        layout.pushBack<float>(3, _normalVertexBuffer);

        _vertexArray->addBuffer(layout);
        _indexBuffer = Common::makeShared<IndexBuffer>(_indices);
        _vertexArray->unBind();

        Common::Performance perf;
        perf.start();
        // Prepare color table SSBO. If no colors provided, fill with default gray per triangle.
        const size_t triangleCount = (_indices.size() / 3);
        if (_colors.size() == 0 && triangleCount > 0)
        {
            _colors.reserve(triangleCount);
            for (size_t i = 0; i < triangleCount; ++i)
            {
                _colors.pushBack(Common::Vector4f(1.0f, 0.5f, 0.5f, 1.0f));
            }
        }

        if (_colors.size() > 0 && _colorSSBO == 0)
        {
            glGenBuffers(1, &_colorSSBO);
        }
        uploadColors();

        perf.stop();

        LOG(INFO) << "Update Color SSBO for RenderFace: "
                  << ", end_ms=" << perf.getEndTimestampMs()
                  << ", elapsed_ms=" << perf.getElapsedMilliseconds();
    }

    void RenderFace::draw(Common::SharedPtr<Shader> shader)
    {
        shader->bind();
        shader->setUniform1i("isPureColor", 0);
        if (_highlightScaleEnabled)
        {
            shader->setUniformMat4f("scale", _highlightScaleMatrix);
        }
        // ensure SSBO is bound before draw
        if (_colorSSBO != 0)
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COLOR_SSBO_BINDING, _colorSSBO);
        }

        _vertexArray->bind();
        _indexBuffer->bind();
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);
        glDrawElements(GL_TRIANGLES, _indexBuffer->getCount(), GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);
        glDisable(GL_POLYGON_OFFSET_FILL);

        _vertexArray->unBind();
        _indexBuffer->unBind();
        shader->unBind();
    }

    void RenderFace::drawHighlight(Common::SharedPtr<Shader> shader, IdxType faceIndex)
    {
        const size_t indexOffset = static_cast<size_t>(faceIndex) * 3;
        if (indexOffset + 2 >= _indices.size() || _colorSSBO == 0)
        {
            return;
        }

        GLint previousDepthFunction = GL_LESS;
        GLboolean previousDepthMask = GL_TRUE;
        glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunction);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -1.0f);

        shader->bind();
        shader->setUniform1i("isPureColor", 1);
        shader->setUniform4f("u_Color", Common::Vector4f(1.0f, 0.28f, 0.04f, 1.0f));
        if (_highlightScaleEnabled)
        {
            shader->setUniformMat4f("scale", _highlightScaleMatrix);
        }
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COLOR_SSBO_BINDING, _colorSSBO);
        _vertexArray->bind();
        _indexBuffer->bind();
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT,
                       reinterpret_cast<const void *>(indexOffset * sizeof(IdxType)));
        _vertexArray->unBind();
        _indexBuffer->unBind();
        shader->setUniform1i("isPureColor", 0);
        shader->unBind();

        glDisable(GL_POLYGON_OFFSET_FILL);
        glDepthMask(previousDepthMask);
        glDepthFunc(static_cast<GLenum>(previousDepthFunction));
    }

    IdxType RenderFace::touch(const PickContext &context)
    {
        if (context.width == 0 || context.height == 0)
        {
            return static_cast<IdxType>(-1);
        }

        Scalar closestDepth = std::numeric_limits<Scalar>::max();
        IdxType touchedIndex = static_cast<IdxType>(-1);

        for (size_t i = 0; i + 2 < _indices.size(); i += 3)
        {
            const IdxType index0 = _indices[i];
            const IdxType index1 = _indices[i + 1];
            const IdxType index2 = _indices[i + 2];
            if (index0 >= _vertices.size() || index1 >= _vertices.size() ||
                index2 >= _vertices.size())
            {
                continue;
            }

            PickUtils::ScreenPoint point0;
            PickUtils::ScreenPoint point1;
            PickUtils::ScreenPoint point2;
            if (!PickUtils::projectPoint(_vertices[index0], context, point0) ||
                !PickUtils::projectPoint(_vertices[index1], context, point1) ||
                !PickUtils::projectPoint(_vertices[index2], context, point2))
            {
                continue;
            }

            const Scalar edge0X = point1.x - point0.x;
            const Scalar edge0Y = point1.y - point0.y;
            const Scalar edge1X = point2.x - point0.x;
            const Scalar edge1Y = point2.y - point0.y;
            const Scalar area = edge0X * edge1Y - edge0Y * edge1X;
            if (std::abs(area) <= 1e-8f)
            {
                continue;
            }

            const Scalar cursorX = context.x - point0.x;
            const Scalar cursorY = context.y - point0.y;
            const Scalar weight1 = (cursorX * edge1Y - cursorY * edge1X) / area;
            const Scalar weight2 = (edge0X * cursorY - edge0Y * cursorX) / area;
            const Scalar weight0 = 1.0f - weight1 - weight2;
            if (weight0 < -1e-6f || weight1 < -1e-6f || weight2 < -1e-6f)
            {
                continue;
            }

            const Scalar depth =
                weight0 * point0.depth + weight1 * point1.depth + weight2 * point2.depth;
            if (depth < closestDepth)
            {
                touchedIndex = static_cast<IdxType>(i / 3);
                closestDepth = depth;
            }
        }

        if (touchedIndex == static_cast<IdxType>(-1) || closestDepth > context.bestDepth + 1e-6f ||
            (std::abs(closestDepth - context.bestDepth) <= 1e-6f &&
             context.bestDistanceSquared <= 0.0f))
        {
            return static_cast<IdxType>(-1);
        }

        context.bestDepth = closestDepth;
        context.bestDistanceSquared = 0.0f;
        return touchedIndex;
    }

    void RenderFace::setNodes(Common::DynamicArray<CoordType> &&nodes)
    {
        _vertices = Common::move(nodes);
        updateHighlightScaleMatrix();
    }
    void RenderFace::setIndices(Common::DynamicArray<IdxType> &&idx)
    {
        _indices = Common::move(idx);
    }

    void RenderFace::setNormals(Common::DynamicArray<CoordType> &&norms)
    {
        _normals = Common::move(norms);
    }

    void RenderFace::setColors(Common::DynamicArray<Common::Vector4f> &&colors)
    {
        _colors = Common::move(colors);
        uploadColors();
    }

    void RenderFace::setColor(const Common::Vector4f &color)
    {
        _colors.clear();
        const size_t triangleCount = _indices.size() / 3;
        _colors.reserve(triangleCount);
        for (size_t i = 0; i < triangleCount; ++i)
        {
            Common::Vector4f triangleColor = color;
            _colors.pushBack(triangleColor);
        }
        uploadColors();
    }

    void RenderFace::setHighlightScale(float scale)
    {
        _highlightScale = scale;
        _highlightScaleEnabled = true;
        updateHighlightScaleMatrix();
    }

    void RenderFace::updateHighlightScaleMatrix()
    {
        _highlightScaleMatrix = Common::Matrix4f::Identity();
        if (!_highlightScaleEnabled)
        {
            return;
        }

        const auto bounds = getBounds();
        if (bounds.isEmpty())
        {
            return;
        }

        const CoordType center = (bounds.min() + bounds.max()) * 0.5f;

        Common::Matrix4f translateToCenter = Common::Matrix4f::Identity();
        translateToCenter[0][3] = center.x();
        translateToCenter[1][3] = center.y();
        translateToCenter[2][3] = center.z();

        Common::Matrix4f scaleByFactor = Common::Matrix4f::Identity();
        scaleByFactor[0][0] = _highlightScale;
        scaleByFactor[1][1] = _highlightScale;
        scaleByFactor[2][2] = _highlightScale;

        Common::Matrix4f translateBack = Common::Matrix4f::Identity();
        translateBack[0][3] = -center.x();
        translateBack[1][3] = -center.y();
        translateBack[2][3] = -center.z();

        _highlightScaleMatrix = translateBack * scaleByFactor * translateToCenter;
    }

    void RenderFace::uploadColors()
    {
        if (_colors.size() == 0 || _colorSSBO == 0)
        {
            return;
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _colorSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, _colors.size() * sizeof(Common::Vector4f),
                     _colors.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COLOR_SSBO_BINDING, _colorSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    Common::BBox<Scalar, 3> RenderFace::getBounds() const
    {
        Common::BBox<Scalar, 3> bounds;
        for (auto it = _vertices.begin(); it.hasNext(); it.next())
        {
            bounds.addPoint(it.data());
        }
        return bounds;
    }

} // namespace EgLab::RE