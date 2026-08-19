#pragma once

#include "Common/HashMap.hpp"
#include "Common/String.hpp"
#include "Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::Common
{
    enum class Return;
}

namespace EgLab::RE
{
    enum class ShaderId;

    struct ShaderProgramSource
    {
        Common::String VertexSource = "";
        Common::String FragmentSource = "";
        Common::String ComputeSource = "";
    };

    // using RendererId = unsigned int;

    class RenderEngineAPI Shader
    {
    public:
        Shader() = default;
        Shader(Common::String&);
        Shader(const ShaderId);
        ~Shader();

        Common::Return bind() const;
        Common::Return unBind() const;

        // template <typename T>
        // Common::Return setUniform(const ShaderId& name, T value);

        Common::Return setUniform1i(const Common::String& name, const int& v0);

        Common::Return setUniform1f(const Common::String& name, const float& v0);

        Common::Return setUniformMat4f(const Common::String& name, const Common::Matrix4f& mat);

        Common::Return setUniform3f(const Common::String& name, const Common::Vector3f& vec);

    private:
        IdType createShader(const ShaderProgramSource&);

        IdType compileShader(unsigned int type, const Common::String& source);

        Common::Return parseShader(Common::String& buffer, ShaderProgramSource&);

        IdType getUniformLocation(const Common::String& name);

    private:
        ShaderId _shader;
        IdType _rendererId;
        Common::HashMap<Common::String, IdType> _uniformLocationCache;
    };

} // namespace EgLab::RE