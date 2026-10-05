#include "RenderEngine/Core/RenderFace.hpp"

#include <GL/glew.h>

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

            for (size_t i = 0; i + 2 < _indices.size(); i += 3)
            {
                const IdxType i0 = _indices[i];
                const IdxType i1 = _indices[i + 1];
                const IdxType i2 = _indices[i + 2];
                if (i0 >= _vertices.size() || i1 >= _vertices.size() || i2 >= _vertices.size())
                {
                    continue;
                }

                CoordType normal = (_vertices[i1] - _vertices[i0]).cross(_vertices[i2] - _vertices[i0]);
                _normals[i0] = _normals[i0] + normal;
                _normals[i1] = _normals[i1] + normal;
                _normals[i2] = _normals[i2] + normal;
            }

            for (size_t i = 0; i < _normals.size(); ++i)
            {
                _normals[i].normalize();
            }
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
    }

    void RenderFace::draw(Common::SharedPtr<Shader> shader)
    {
        shader->bind();
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

    void RenderFace::setNodes(Common::DynamicArray<CoordType> &&nodes)
    {
        _vertices = Common::move(nodes);
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

    void RenderFace::setColor(const Common::Vector4f& color)
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