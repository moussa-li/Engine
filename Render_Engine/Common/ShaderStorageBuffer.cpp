
#include "Renderer.h"
#include "ShaderStorageBuffer.h"

ShaderStorageBuffer::ShaderStorageBuffer(unsigned int size, unsigned int bindingId)
{
    GLCall(glGenBuffers(1, &m_RendererId));
    Bind();
    GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_COPY));
    GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingId, m_RendererId));
    m_BindingId = bindingId;
    m_Size = size;
    m_Stride = size;
    UnBind();
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
    glDeleteBuffers(1, &m_RendererId);
}

/// <summary>
/// input the data pointer to the buffer data  , if over size re-init the buffer size
/// </summary>
/// <param name="point"> input data : the pointer for what you want to input </param>
/// <param name="size"> input data : how many data do you want to input </param>
void ShaderStorageBuffer::InputData(void* point, unsigned int size)
{
    Bind();
    if (size > m_Size)
    {
        GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, size, point, GL_DYNAMIC_COPY));
        m_Size = size;
    }
    void * bufferMap = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(bufferMap, point, size);
    m_Stride = size;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    UnBind();

}

/// <summary>
/// Get the buffer's data for a pointer
/// </summary>
/// <param name="point"> output data : return a pointer to the buffer data</param>
/// <param name="size"> output data : return the size of buffer </param>
void ShaderStorageBuffer::OutputData(void *point, unsigned int &size)
{
    Bind();
    void* bufferMap = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    memcpy(point, bufferMap, m_Stride);
    size = m_Stride;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    UnBind();

    
}

void ShaderStorageBuffer::OutputData(void *point)
{
    Bind();
    void* bufferMap = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    memcpy(point, bufferMap, m_Stride);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    UnBind();
}

void ShaderStorageBuffer::Bind()
{
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererId));
    //GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingId, m_RendererId));
}

void ShaderStorageBuffer::UnBind()
{
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

