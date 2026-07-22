#include "RenderEngine/Core/Shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Common/Return.hpp"
#include "Common/String.hpp"
#include "Common/StringLineIterator.hpp"
#include "Core/ShaderLib.hpp"
#include "GLWrapper.hpp"

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
        glUseProgram(_rendererId);
        return ret;
    }

    Return Shader::unBind() const
    {
        glUseProgram(0);
        Return ret = Return::Succeed;
        return ret;
    }

    IdType Shader::compileShader(unsigned int type, const String& source)
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

        return Return::NotImplacementYet;
    }

    Return Shader::setUniform1i(const String& name, const int& v0)
    {
        // LOG(INFO) << "value name : " << name;
        glUniform1iv(getUniformLocation(name), 1, &v0);
        return Return::Succeed;
    }

    Return Shader::setUniform1f(const String& name, const float& v0)
    {
        // LOG(INFO) << "value name : " << name;
        glUniform1fv(getUniformLocation(name), 1, &v0);
        return Return::Succeed;
    }

    Return Shader::setUniformMat4f(const String& name, const Matrix4f& mat)
    {
        // LOG(INFO) << "value name : " << name;
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_TRUE, mat.data());
        return Return::Succeed;
    }

    Return Shader::setUniform3f(const String& name, const Vector3f& vec)
    {
        // LOG(INFO) << "value name : " << name;
        glUniform3fv(getUniformLocation(name), 1, vec.data());
        return Return::Succeed;
    }

    IdType Shader::getUniformLocation(const String& name)
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
} // namespace EgLab