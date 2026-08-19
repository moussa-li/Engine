#include <GL/glew.h>

#include "Common/DynamicArray.hpp"
#include "Common/Log.hpp"
#include "VertexBuffer.hpp"

#define ASSERT(x) \
    if (!(x)) __debugbreak();

#define GLCall(x)              \
    EgLab::RE::GLClearError(); \
    x;                         \
    ASSERT(EgLab::RE::GLLogCall(#x, __FILE__, __LINE__))

#define GL_CHECK()                                                                          \
    {                                                                                       \
        GLenum err = glGetError();                                                          \
        if (err != GL_NO_ERROR)                                                             \
        {                                                                                   \
            LOG(ERROR) << "OpenGL Error: " << err << " at " << __FILE__ << ":" << __LINE__; \
        }                                                                                   \
    }

namespace EgLab::RE
{

    void GLClearError();

    bool GLLogCall(const char* function, const char* file, int line);

} // namespace EgLab::RE