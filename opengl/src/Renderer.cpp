#include "Renderer.h"
#include <iostream>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char * function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ") : " << function << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

void Renderer::Clear()
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT);)
}
void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
{

    shader.Bind();
    //shader.SetUniform4f("u_Color", r, 0.3f, 0.2f, 1.0f);

	va.Bind();
	ib.Bind();


}
