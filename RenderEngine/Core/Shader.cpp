#include "RenderEngine/Core/Shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Common/Return.hpp"
#include "Common/String.hpp"
#include "Common/StringLineIterator.hpp"
#include "GLWrapper.hpp"

namespace EgLab::RE
{
    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1,
        COMPUTE = 2
    };

    Shader::Shader(Common::String& buffer) : _rendererId(0)
    {
        ShaderProgramSource source;
        parseShader(buffer, source);

        _rendererId = createShader(source);
    }

    Shader::~Shader()
    {
    }

    Common::Return Shader::bind() const
    {
        Common::Return ret = Common::Return::Succeed;
        glUseProgram(_rendererId);
        return ret;
    }

    Common::Return Shader::unBind() const
    {
        glUseProgram(0);
        Common::Return ret = Common::Return::Succeed;
        return ret;
    }

    IdType Shader::compileShader(unsigned int type, const Common::String& source)
    {
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);
        GL_CHECK();

        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            int length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            char* msg = new char[length];
            glGetShaderInfoLog(id, length, &length, msg);
            LOG(ERROR) << "Failed to comile [ " << type << "] : " << msg;
            GL_CHECK();
            glDeleteShader(id);
            return 0;
        }
        return id;
    }

    IdType Shader::createShader(const ShaderProgramSource& source)
    {
        IdType program = glCreateProgram();
        if (source.VertexSource != "")
        {
            IdType vs = compileShader(GL_VERTEX_SHADER, source.VertexSource);
            glAttachShader(program, vs);
            glDeleteShader(vs);
        }

        if (source.FragmentSource != "")
        {
            IdType fs = compileShader(GL_FRAGMENT_SHADER, source.FragmentSource);
            glAttachShader(program, fs);
            glDeleteShader(fs);
        }

        if (source.ComputeSource != "")
        {
            IdType cs = compileShader(GL_COMPUTE_SHADER, source.ComputeSource);
            glAttachShader(program, cs);
            glDeleteShader(cs);
        }
        glLinkProgram(program);
        glValidateProgram(program);
        return program;
    }

    Common::Return Shader::parseShader(Common::String& buffer, ShaderProgramSource& source)
    {
        Common::StringLineIterator it(buffer);

        Common::DynamicArray<Common::String> lines;
        ShaderType type = ShaderType::NONE;
        for (; it.hasNext(); ++it)
        {
            const char* str;
            size_t len;
            it.getString(str, len);
            Common::String content(str, len);
            if (content == "#shader vertex\r\n")
            {
                type = ShaderType::VERTEX;
                continue;
            }
            else if (content == "#shader fragment\r\n")
            {
                type = ShaderType::FRAGMENT;
                continue;
            }
            else if (content == "#shader compute\r\n")
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

        return Common::Return::NotImplacementYet;
    }

    Common::Return Shader::setUniform1i(const Common::String& name, const int& v0)
    {
        // LOG(INFO) << "value name : " << name;
        glUniform1iv(getUniformLocation(name), 1, &v0);
        return Common::Return::Succeed;
    }

    Common::Return Shader::setUniform1f(const Common::String& name, const float& v0)
    {
        // LOG(INFO) << "value name : " << name;
        glUniform1fv(getUniformLocation(name), 1, &v0);
        return Common::Return::Succeed;
    }

    Common::Return Shader::setUniformMat4f(const Common::String& name, const Common::Matrix4f& mat)
    {
        // LOG(INFO) << "value name : " << name;
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_TRUE, mat.data());
        return Common::Return::Succeed;
    }

    Common::Return Shader::setUniform3f(const Common::String& name, const Common::Vector3f& vec)
    {
        // LOG(INFO) << "value name : " << name;
        glUniform3fv(getUniformLocation(name), 1, vec.data());
        return Common::Return::Succeed;
    }

    IdType Shader::getUniformLocation(const Common::String& name)
    {
        auto it = _uniformLocationCache.find(name);
        if (it != _uniformLocationCache.end())
        {
            return (*it).second;
        }

        IdType location = glGetUniformLocation(_rendererId, name.c_str());
        if (location == -1)
        {
            LOG(WARNING) << "uniform " << name.c_str() << " doesn't exist!";
        }

        _uniformLocationCache[name] = location;

        return location;
    }
} // namespace EgLab::RE