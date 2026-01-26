#include "RenderEngine/Core/Shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Common/Return.hpp"
#include "Common/String.hpp"
#include "Common/StringLineIterator.hpp"
#include "Core/ShaderLib.hpp"

namespace EgLab
{

    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1,
        COMPUTE = 2
    };

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
        String buffer;
        ShaderLib::instance().getBasicShader(buffer);
        StringLineIterator it(buffer);

        EgLab::DynamicArray<EgLab::String> lines;
        ShaderType type = ShaderType::NONE;
        for (; it.hasNext(); ++it)
        {
            const char* str;
            size_t len;
            it.getString(str, len);
            EgLab::String content(str, len);
            if (content == "#shader vertex")
            {
                type = ShaderType::VERTEX;
                continue;
            }
            else if (content == "#shader fragment")
            {
                type = ShaderType::FRAGMENT;
                continue;
            }
            else if (content == "#shader compute")
            {
                type = ShaderType::COMPUTE;
                continue;
            }

            switch (type)
            {
            case ShaderType::VERTEX:
                source.VertexSource += content;
                break;
            case ShaderType::FRAGMENT:
                source.FragmentSource += content;
                break;
            case ShaderType::COMPUTE:
                source.ComputeSource += content;
                break;
            default:
                LOG(ERROR) << "this line is unknown shader line!";
            }
        }

        return Return::NotImplacementYet;
    }
} // namespace EgLab