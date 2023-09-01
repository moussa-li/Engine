#include "Shader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "Print.h"

Shader::Shader(const std::string& filepath)
	: m_FilePath(filepath), m_RendererId(0)
{
    //ShaderProgramSource source = ParseShader(m_FilePath);
    m_RendererId = CreateShader(ParseShader(m_FilePath));
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererId));
}



ShaderProgramSource Shader::ParseShader(const std::string& filepath) {
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1, COMPUTE = 2
    };

    std::string line;
    std::stringstream ss[3];
    ShaderType type;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                //set mode to vertex
            {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
                // set mode to fragment
            {
                type = ShaderType::FRAGMENT;
            }
            else if (line.find("compute") != std::string::npos)
                // set mode to compute
            {
                type = ShaderType::COMPUTE;
            }
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }
    return { ss[0].str(), ss[1].str(), ss[2].str() };
}
unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {

    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if(result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = new char[length];
        //char* message = (char*)alloca(sizeof(char) * length);
        glGetShaderInfoLog(id, length, &length, message);

        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

//int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
int Shader::CreateShader(ShaderProgramSource source) {
    unsigned int program = glCreateProgram();
    if (source.VertexSource != "")
    {
        unsigned int vs = CompileShader(GL_VERTEX_SHADER, source.VertexSource);
        glAttachShader(program, vs);
        glDeleteShader(vs);
    }
    if (source.FragmentSource != "")
    {
        unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, source.FragmentSource);
        glAttachShader(program, fs);
        glDeleteShader(fs);
    }
    if (source.ComputeSource != "")
    {
        unsigned int cs = CompileShader(GL_COMPUTE_SHADER, source.ComputeSource);
        glAttachShader(program, cs);
        glDeleteShader(cs);

    }
    glLinkProgram(program);
    glValidateProgram(program);


    return program;
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererId));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}
void Shader::SetUniform1i(const std::string& name, int value)
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
}
void Shader::SetUniform1f(const std::string& name, float value)
{
    GLCall(glUniform1f(GetUniformLocation(name), value));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void Shader::SetUniform3f(const std::string& name, Eigen::Vector3f v)
{
    GLCall(glUniform3fv(GetUniformLocation(name), 1, v.data()));
}

void Shader::SetUniformMat4f(const std::string& name, const Eigen::Matrix4f& matrix)
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, matrix.data()));
}

void Shader::SetUniformMat3f(const std::string& name, const Eigen::Matrix3f& matrix)
{
    GLCall(glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, matrix.data()));
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    GLCall(int location = glGetUniformLocation(m_RendererId, name.c_str()));
    if (location == -1)
        std::cout << "warning: uniform'" << name << "' doesn't exist!" << std::endl;

    m_UniformLocationCache[name] = location;
    return location;
}

