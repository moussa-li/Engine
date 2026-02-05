#pragma once

#include "Common/HashMap.hpp"
#include "Common/String.hpp"
#include "Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab
{
    class String;
    enum class ShaderId;
    enum class Return;

    struct ShaderProgramSource
    {
        String VertexSource = "";
        String FragmentSource = "";
        String ComputeSource = "";
    };

    // using RendererId = unsigned int;

    class RenderEngineAPI Shader
    {
    public:
        Shader() = default;
        Shader(const ShaderId);
        ~Shader();

        Return bind() const;
        Return unBind() const;

        // template <typename T>
        // Return setUniform(const ShaderId& name, T value);

        Return setUniform1i(const String& name, const int& v0);

        Return setUniform1f(const String& name, const float& v0);

        Return setUniformMat4f(const String& name, const Matrix4f& mat);

    private:
        IdType createShader(const ShaderProgramSource&);

        IdType compileShader(unsigned int type, const String& source);

        Return parseShader(ShaderProgramSource&);

        IdType getUniformLocation(const String& name);

    private:
        ShaderId _shader;
        IdType _rendererId;
        HashMap<String, IdType> _uniformLocationCache;
    };

} // namespace EgLab