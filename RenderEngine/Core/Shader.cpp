#include "RenderEngine/Core/Shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Common/Return.hpp"
#include "Common/String.hpp"
#include "Core/ShaderLib.hpp"

namespace EgLab
{
    Shader::Shader(const ShaderId shader) : _shader(shader), _rendererId(0)
    {
        ShaderProgramSource source;
        parseShader(source);

        _rendererId = createShader(source);
    }

    Shader::~Shader()
    {
    }

    Return Shader::bind() const
    {
        Return ret = Return::Succeed;
        return ret;
    }

    Return Shader::unBind() const
    {
        Return ret = Return::Succeed;
        return ret;
    }

    IdType Shader::createShader(const ShaderProgramSource& source)
    {
        return 0;
    }

    Return Shader::parseShader(ShaderProgramSource& source)
    {
        return Return::NotImplacementYet;
    }
} // namespace EgLab