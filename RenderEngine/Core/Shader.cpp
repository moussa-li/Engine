#include "RenderEngine/Core/Shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Common/Assets.hpp"
#include "Common/Return.hpp"
#include "Common/String.hpp"

namespace EgLab
{
    Shader::Shader(const AssetId shader) : _shader(shader), _rendererId(0)
    {
        ShaderProgramSource source;
        parseShader(source);

        _rendererId = createShader(source);
    }

    Shader::~Shader() {}

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

    RendererId Shader::createShader(const ShaderProgramSource& source) { return 0; }

    Return Shader::parseShader(ShaderProgramSource& source) { return Return::NotImplacementYet; }
} // namespace EgLab