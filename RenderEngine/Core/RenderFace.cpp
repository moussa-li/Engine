#include "RenderEngine/Core/RenderFace.hpp"

#include <GL/glew.h>

#include "Common/Utils.hpp"
#include "RenderEngine/Core/Shader.hpp"
#include "RenderEngine/Core/VertexBufferLayout.hpp"

namespace EgLab::RE
{
    RenderFace::~RenderFace()
    {
    }

    void RenderFace::setup()
    {
        _vertexArray = Common::makeShared<VertexArray>();

        VertexBufferLayout layout;
        _vertexBuffer = Common::makeShared<VertexBuffer>(_vertices);
        layout.pushBack<float>(3, _vertexBuffer);

        _normalVertexBuffer = Common::makeShared<VertexBuffer>(_normals);
        layout.pushBack<float>(3, _normalVertexBuffer);

        _vertexArray->addBuffer(layout);
        _indexBuffer = Common::makeShared<IndexBuffer>(_indices);
        _vertexArray->unBind();
    }

    void RenderFace::draw(Common::SharedPtr<Shader> shader)
    {
        shader->bind();
        _vertexArray->bind();
        _indexBuffer->bind();
        glDrawElements(GL_TRIANGLES, _indexBuffer->getCount(), GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);
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

} // namespace EgLab::RE