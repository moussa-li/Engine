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

    void RenderNode::setNodes(Common::DynamicArray<CoordType>&& nodes)
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
        _vertexArray->bind();
        glDrawArrays(GL_POINTS, 0, _vertices.size());
        glDepthMask(GL_TRUE);
        glEnable(GL_PROGRAM_POINT_SIZE);
        _vertexArray->unBind();
        shader->unBind();
    }
} // namespace EgLab::RE
