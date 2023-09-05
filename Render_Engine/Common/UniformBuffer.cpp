
#include "Renderer.h"
#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(unsigned int size, unsigned int bindingId)
{
    GLCall(glGenBuffers(1, &m_RendererId));
    Bind();
    GLCall(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_COPY));
    GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, bindingId, m_RendererId));
    m_BindingId = bindingId;
    m_Size = size;
    m_Stride = 0;
    UnBind();
}

/// <summary>
/// intput the data pointer to the buffer data  , if over size reinit the buffer size
/// </summary>
/// <param name="point"> input data : the pointer for what you want to input </param>
/// <param name="size"> input data : how many data do you want to input </param>
void UniformBuffer::InputData(void* point, unsigned int size)
{
    Bind();
    if (size > m_Size)
    {
        GLCall(glBufferData(GL_UNIFORM_BUFFER, size, point, GL_STATIC_COPY));
        m_Size = size;
    }
    void * bufferMap = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    memcpy(bufferMap, point, size);
    m_Stride += size;
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    UnBind();

}

/// <summary>
/// Get the buffer's data for a pointer
/// </summary>
/// <param name="point"> output data : return a pointer to the buffer data</param>
/// <param name="size"> output data : return the size of buffer </param>
void UniformBuffer::OutputData(void *point, unsigned int &size)
{
    Bind();
    void* bufferMap = glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_ONLY);
    memcpy(point, bufferMap, m_Stride);
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    UnBind();

    
}

void UniformBuffer::Bind()
{
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_RendererId));
}

void UniformBuffer::UnBind()
{
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

