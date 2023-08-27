#include "VertexBuffer.h"

#include "Renderer.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
    GLCall(glGenBuffers(1, &m_RendererId));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
     
}


VertexBuffer::VertexBuffer(std::vector<Vertex> vertices)
{
    GLCall(glGenBuffers(1, &m_RendererId));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() *  sizeof(Vertex) , &vertices[0], GL_STATIC_DRAW));
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