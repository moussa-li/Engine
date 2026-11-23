#include "RenderEngine/Core/ReferencePlane.hpp"

#include <GL/glew.h>

#include <algorithm>
#include <cmath>

#include "RenderEngine/Core/Shader.hpp"
#include "RenderEngine/Core/VertexArray.hpp"
#include "RenderEngine/Core/VertexBuffer.hpp"
#include "RenderEngine/Core/VertexBufferLayout.hpp"

namespace EgLab::RE
{
    ReferencePlane::ReferencePlane()
    {
    }

    ReferencePlane::~ReferencePlane()
    {
    }

    const ReferencePlaneSettings &ReferencePlane::getSettings() const
    {
        return _settings;
    }

    void ReferencePlane::setSettings(const ReferencePlaneSettings &settings)
    {
        _settings = settings;
    }

    Common::BBox<Scalar, 3> ReferencePlane::getRenderBounds(
        const Common::BBox<Scalar, 3> &sceneBounds) const
    {
        if (sceneBounds.isEmpty() || !_settings.enabled)
        {
            return sceneBounds;
        }

        // Every plane vertex stays within the scene bounds scaled about its centre: in-plane
        // extents are scaled directly and the back-face placement shifts each plane along its
        // normal to the scaled minimum. A single scaled box therefore covers all three planes.
        const Scalar scale = std::max(_settings.scale, 1.0f);
        const CoordType boundsMin = sceneBounds.min();
        const CoordType boundsMax = sceneBounds.max();

        CoordType expandedMin;
        CoordType expandedMax;
        for (size_t dim = 0; dim < 3; ++dim)
        {
            const Scalar center = (boundsMin[dim] + boundsMax[dim]) * 0.5f;
            const Scalar half = (boundsMax[dim] - boundsMin[dim]) * 0.5f * scale;
            expandedMin[dim] = center - half;
            expandedMax[dim] = center + half;
        }

        Common::BBox<Scalar, 3> result;
        result.addPoint(expandedMin);
        result.addPoint(expandedMax);
        return result;
    }

    Scalar ReferencePlane::computeGridStep(Scalar maxExtent)
    {
        if (maxExtent <= 0.0f)
        {
            return 1.0f;
        }

        const Scalar raw = maxExtent / 10.0f;
        const Scalar exponent = std::floor(std::log10(raw));
        const Scalar magnitude = std::pow(10.0f, exponent);
        const Scalar normalized = raw / magnitude;

        Scalar factor = 10.0f;
        if (normalized <= 1.0f)
        {
            factor = 1.0f;
        }
        else if (normalized <= 2.0f)
        {
            factor = 2.0f;
        }
        else if (normalized <= 5.0f)
        {
            factor = 5.0f;
        }
        return factor * magnitude;
    }

    bool ReferencePlane::boundsChanged(const Common::BBox<Scalar, 3> &bounds) const
    {
        if (!_hasCachedBounds)
        {
            return true;
        }

        const CoordType boundsMin = bounds.min();
        const CoordType boundsMax = bounds.max();
        for (size_t dim = 0; dim < 3; ++dim)
        {
            if (boundsMin[dim] != _cachedMin[dim] || boundsMax[dim] != _cachedMax[dim])
            {
                return true;
            }
        }

        return false;
    }

    void ReferencePlane::rebuild(const Common::BBox<Scalar, 3> &bounds)
    {
        _quadVertices.clear();
        _lineVertices.clear();
        for (int plane = 0; plane < 3; ++plane)
        {
            _quadRanges[plane] = PlaneRange{};
            _lineRanges[plane] = PlaneRange{};
        }

        const CoordType boundsMin = bounds.min();
        const CoordType boundsMax = bounds.max();
        CoordType center;
        for (size_t dim = 0; dim < 3; ++dim)
        {
            center[dim] = (boundsMin[dim] + boundsMax[dim]) * 0.5f;
        }

        // Plane 0 has the X normal (YZ), plane 1 the Y normal (XZ), plane 2 the Z normal (XY).
        for (int normal = 0; normal < 3; ++normal)
        {
            const int axisA = (normal + 1) % 3;
            const int axisB = (normal + 2) % 3;

            const Scalar aMin = boundsMin[axisA];
            const Scalar aMax = boundsMax[axisA];
            const Scalar bMin = boundsMin[axisB];
            const Scalar bMax = boundsMax[axisB];

            // Scale the in-plane extents about the bounds center. The plane centers are moved
            // along their normals by the placement setting, which keeps the intersection of
            // the three planes at a bounding-box vertex in back-face mode.
            const Scalar scale = _settings.scale > 0.01f ? _settings.scale : 0.01f;
            const Scalar aCenter = (aMin + aMax) * 0.5f;
            const Scalar bCenter = (bMin + bMax) * 0.5f;
            const Scalar aHalf = (aMax - aMin) * 0.5f * scale;
            const Scalar bHalf = (bMax - bMin) * 0.5f * scale;
            const Scalar scaledAMin = aCenter - aHalf;
            const Scalar scaledAMax = aCenter + aHalf;
            const Scalar scaledBMin = bCenter - bHalf;
            const Scalar scaledBMax = bCenter + bHalf;

            const Scalar cCenter = (boundsMin[normal] + boundsMax[normal]) * 0.5f;
            const Scalar cHalf = (boundsMax[normal] - boundsMin[normal]) * 0.5f * scale;
            const Scalar scaledCMin = cCenter - cHalf;

            const Scalar normalCoord =
                _settings.placement == ReferencePlanePlacement::BoundingBoxBackFaces
                    ? scaledCMin
                    : center[normal];

            const auto makePoint = [normalCoord, normal, axisA, axisB](Scalar a, Scalar b) {
                CoordType point;
                point[normal] = normalCoord;
                point[axisA] = a;
                point[axisB] = b;
                return point;
            };

            const CoordType v00 = makePoint(scaledAMin, scaledBMin);
            const CoordType v10 = makePoint(scaledAMax, scaledBMin);
            const CoordType v11 = makePoint(scaledAMax, scaledBMax);
            const CoordType v01 = makePoint(scaledAMin, scaledBMax);

            _quadRanges[normal].offset = _quadVertices.size();
            _quadVertices.pushBack(v00);
            _quadVertices.pushBack(v10);
            _quadVertices.pushBack(v11);
            _quadVertices.pushBack(v00);
            _quadVertices.pushBack(v11);
            _quadVertices.pushBack(v01);
            _quadRanges[normal].count = _quadVertices.size() - _quadRanges[normal].offset;

            if (!_settings.showGrid)
            {
                continue;
            }

            _lineRanges[normal].offset = _lineVertices.size();

            // Rectangle border.
            _lineVertices.pushBack(v00);
            _lineVertices.pushBack(v10);
            _lineVertices.pushBack(v10);
            _lineVertices.pushBack(v11);
            _lineVertices.pushBack(v11);
            _lineVertices.pushBack(v01);
            _lineVertices.pushBack(v01);
            _lineVertices.pushBack(v00);

            const Scalar extentA = scaledAMax - scaledAMin;
            const Scalar extentB = scaledBMax - scaledBMin;
            const Scalar step = computeGridStep(extentA > extentB ? extentA : extentB);
            if (step > 0.0f)
            {
                const long long firstA = static_cast<long long>(std::ceil(scaledAMin / step));
                const long long lastA = static_cast<long long>(std::floor(scaledAMax / step));
                for (long long index = firstA; index <= lastA; ++index)
                {
                    const Scalar a = static_cast<Scalar>(index) * step;
                    _lineVertices.pushBack(makePoint(a, scaledBMin));
                    _lineVertices.pushBack(makePoint(a, scaledBMax));
                }

                const long long firstB = static_cast<long long>(std::ceil(scaledBMin / step));
                const long long lastB = static_cast<long long>(std::floor(scaledBMax / step));
                for (long long index = firstB; index <= lastB; ++index)
                {
                    const Scalar b = static_cast<Scalar>(index) * step;
                    _lineVertices.pushBack(makePoint(scaledAMin, b));
                    _lineVertices.pushBack(makePoint(scaledAMax, b));
                }
            }

            _lineRanges[normal].count = _lineVertices.size() - _lineRanges[normal].offset;
        }

        uploadQuadGeometry();
        uploadLineGeometry();

        _cachedMin = boundsMin;
        _cachedMax = boundsMax;
        _cachedShowGrid = _settings.showGrid;
        _cachedPlacement = _settings.placement;
        _cachedScale = _settings.scale;
        _hasCachedBounds = true;
    }

    void ReferencePlane::uploadQuadGeometry()
    {
        if (_quadVertices.empty())
        {
            return;
        }

        const size_t byteSize = _quadVertices.size() * sizeof(CoordType);
        if (_quadVertexArray == nullptr)
        {
            _quadVertexArray = Common::makeShared<VertexArray>();
            _quadVertexBuffer = Common::makeShared<VertexBuffer>(
                static_cast<const void *>(_quadVertices.data()), byteSize);
            VertexBufferLayout layout;
            layout.pushBack<float>(3, _quadVertexBuffer);
            _quadVertexArray->addBuffer(layout);
        }
        else
        {
            _quadVertexBuffer->setData(static_cast<const void *>(_quadVertices.data()), byteSize);
        }
    }

    void ReferencePlane::uploadLineGeometry()
    {
        if (_lineVertices.empty())
        {
            return;
        }

        const size_t byteSize = _lineVertices.size() * sizeof(CoordType);
        if (_lineVertexArray == nullptr)
        {
            _lineVertexArray = Common::makeShared<VertexArray>();
            _lineVertexBuffer = Common::makeShared<VertexBuffer>(
                static_cast<const void *>(_lineVertices.data()), byteSize);
            VertexBufferLayout layout;
            layout.pushBack<float>(3, _lineVertexBuffer);
            _lineVertexArray->addBuffer(layout);
        }
        else
        {
            _lineVertexBuffer->setData(static_cast<const void *>(_lineVertices.data()), byteSize);
        }
    }

    void ReferencePlane::draw(const Common::BBox<Scalar, 3> &bounds,
                              const Common::SharedPtr<Shader> &shader)
    {
        if (!_settings.enabled || bounds.isEmpty() || shader == nullptr)
        {
            return;
        }

        if (boundsChanged(bounds) || _cachedShowGrid != _settings.showGrid ||
            _cachedPlacement != _settings.placement || _cachedScale != _settings.scale)
        {
            rebuild(bounds);
        }

        if (_quadVertexArray == nullptr)
        {
            return;
        }

        GLboolean previousBlend = glIsEnabled(GL_BLEND);
        GLboolean previousDepthMask = GL_TRUE;
        GLint previousBlendSrcRgb = GL_ONE;
        GLint previousBlendDstRgb = GL_ZERO;
        GLint previousBlendSrcAlpha = GL_ONE;
        GLint previousBlendDstAlpha = GL_ZERO;
        GLfloat previousLineWidth = 1.0f;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask);
        glGetIntegerv(GL_BLEND_SRC_RGB, &previousBlendSrcRgb);
        glGetIntegerv(GL_BLEND_DST_RGB, &previousBlendDstRgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &previousBlendSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &previousBlendDstAlpha);
        glGetFloatv(GL_LINE_WIDTH, &previousLineWidth);

        // Blend the planes but never write depth: the scene is drawn afterwards and must
        // always cover them, so the reference planes can never occlude the model.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        shader->bind();

        _quadVertexArray->bind();
        for (int plane = 0; plane < 3; ++plane)
        {
            const PlaneRange &range = _quadRanges[plane];
            if (range.count == 0)
            {
                continue;
            }
            shader->setUniform4f("u_Color", _settings.colors[plane]);
            glDrawArrays(GL_TRIANGLES, static_cast<GLint>(range.offset),
                         static_cast<GLsizei>(range.count));
        }
        _quadVertexArray->unBind();

        if (_settings.showGrid && _lineVertexArray != nullptr)
        {
            glLineWidth(1.0f);
            _lineVertexArray->bind();
            for (int plane = 0; plane < 3; ++plane)
            {
                const PlaneRange &range = _lineRanges[plane];
                if (range.count == 0)
                {
                    continue;
                }
                // Grid lines use a more opaque version of the plane color so they stay
                // readable against the low-alpha plane fill.
                Common::Vector4f gridColor = _settings.colors[plane];
                gridColor[3] = gridColor[3] > 0.6f ? gridColor[3] : 0.6f;
                shader->setUniform4f("u_Color", gridColor);
                glDrawArrays(GL_LINES, static_cast<GLint>(range.offset),
                             static_cast<GLsizei>(range.count));
            }
            _lineVertexArray->unBind();
            glLineWidth(previousLineWidth);
        }

        shader->unBind();

        glDepthMask(previousDepthMask);
        glBlendFuncSeparate(previousBlendSrcRgb, previousBlendDstRgb, previousBlendSrcAlpha,
                            previousBlendDstAlpha);
        if (previousBlend)
        {
            glEnable(GL_BLEND);
        }
        else
        {
            glDisable(GL_BLEND);
        }
    }
} // namespace EgLab::RE
