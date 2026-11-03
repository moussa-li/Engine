#include <GL/glew.h>

#include "Common/DynamicArray.hpp"
#include "Common/Log.hpp"
#include "VertexBuffer.hpp"

/// Asserts that the wrapped condition is true.
#define ASSERT(x) \
    if (!(x)) __debugbreak();

/// Clears prior OpenGL errors, executes the expression, and logs its failure.
#define GLCall(x)              \
    EgLab::RE::GLClearError(); \
    x;                         \
    ASSERT(EgLab::RE::GLLogCall(#x, __FILE__, __LINE__))

/// Logs the current OpenGL error, if one is present.
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

    /// Clears all pending OpenGL errors.
    void GLClearError();

    /**
     * @brief Logs the result of an OpenGL call.
     * @param function Expression or function name being checked.
     * @param file Source file containing the call.
     * @param line Source line containing the call.
     * @return true when no OpenGL error is reported.
     */
    bool GLLogCall(const char* function, const char* file, int line);

} // namespace EgLab::RE