#include <GL/glew.h>

#include "Common/DynamicArray.hpp"
#include "VertexBuffer.hpp"

#define ASSERT(x) \
    if (!(x)) __debugbreak();

#define GLCall(x)          \
    EgLab::GLClearError(); \
    x;                     \
    ASSERT(EgLab::GLLogCall(#x, __FILE__, __LINE__))

namespace EgLab
{

    void GLClearError();

    bool GLLogCall(const char* function, const char* file, int line);

} // namespace EgLab