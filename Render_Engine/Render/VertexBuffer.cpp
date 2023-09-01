#include "VertexBuffer.h"

#include "Renderer.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
    GLCall(glGenBuffers(1, &m_RendererId));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
     
}


//VertexBuffer::VertexBuffer(std::vector<Vertex> vertices)
//{
//    GLCall(glGenBuffers(1, &m_RendererId));
//    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
//    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() *  sizeof(Vertex) , &vertices[0], GL_STATIC_DRAW));
//}

VertexBuffer::VertexBuffer(std::vector<Eigen::Vector3f> *v)
{
    GLCall(glGenBuffers(1, &m_RendererId));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
    GLCall(glBufferData(GL_ARRAY_BUFFER, v->size() *  sizeof(float) * 3 , &(*v)[0], GL_STATIC_DRAW));
    m_Data = (void*)v;
    Size += v->size() * sizeof(float) * 3;
}

VertexBuffer::VertexBuffer(std::vector<Eigen::Vector2f> *v)
{
    GLCall(glGenBuffers(1, &m_RendererId));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
    GLCall(glBufferData(GL_ARRAY_BUFFER, v->size() *  sizeof(float) * 2 , &(*v)[0], GL_STATIC_DRAW));
    m_Data = (void*)v;
    Size += v->size() * sizeof(float) * 2;
}

VertexBuffer::~VertexBuffer()
{
    GLCall(glDeleteBuffers(1, &m_RendererId));
}

void VertexBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));

}

void VertexBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

}