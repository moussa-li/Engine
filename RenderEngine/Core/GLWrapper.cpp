#include <GL/glew.h>

#include <iostream>

namespace EgLab::RE
{

    void GLClearError()
    {
        constexpr unsigned int maxErrorReads = 32;
        for (unsigned int attempt = 0; attempt < maxErrorReads; ++attempt)
        {
            if (glGetError() == GL_NO_ERROR)
            {
                return;
            }
        }
    }

    bool GLLogCall(const char* function, const char* file, int line)
    {
        while (GLenum error = glGetError())
        {
            std::cout << "[OpenGL Error] (" << error << ") : " << function << file << ":" << line
                      << std::endl;
            return false;
        }
        return true;
    }

} // namespace EgLab::RE