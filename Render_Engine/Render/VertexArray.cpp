#include "VertexArray.h"
#include "VertexBufferLayout.h"

#include "Renderer.h"


VertexArray::VertexArray() 
{
    GLCall(glGenVertexArrays(1, &m_RendererId));
}
VertexArray::~VertexArray() 
{
    GLCall(glDeleteVertexArrays(1, &m_RendererId));
}

void VertexArray::AddBuffer(const VertexBufferLayout& layout) 
{
    Bind();
    const auto& elements = layout.GetElement();
    for (size_t i = 0; i < elements.size(); i++)
    {
        const auto& element = elements[i];
        element.VBO->Bind();
        GLCall(glEnableVertexAttribArray(i));
        GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, 0 , NULL));
    }

}

void VertexArray::Bind() const
{
    GLCall(glBindVertexArray(m_RendererId));
}
void VertexArray::Unbind() const
{
    GLCall(glBindVertexArray(0));
}