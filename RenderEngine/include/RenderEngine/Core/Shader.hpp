#pragma once

#include "Common/HashMap.hpp"
#include "Common/Return.hpp"
#include "Common/String.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    enum class ShaderId;

    struct ShaderProgramSource
    {
        /// GLSL vertex shader source.
        Common::String VertexSource = "";
        /// GLSL fragment shader source.
        Common::String FragmentSource = "";
        /// GLSL compute shader source.
        Common::String ComputeSource = "";
    };

    // using RendererId = unsigned int;

    /// Compiles, binds, and configures an OpenGL shader program.
    class RenderEngineAPI Shader
    {
    public:
        /// @brief Creates an uninitialized shader object.
        Shader() = default;
        /// @brief Creates a shader program from source text.
        /// @param source Shader source buffer.
        Shader(Common::String&);
        /// @brief Creates a shader program identified by a registered shader ID.
        /// @param shader Shader identifier to load.
        Shader(const ShaderId);
        /// @brief Releases the shader program.
        ~Shader();

        /// @brief Binds this shader program for rendering.
        Common::Return bind() const;
        /// @brief Unbinds the current shader program.
        Common::Return unBind() const;

        // template <typename T>
        // Common::Return setUniform(const ShaderId& name, T value);

        /// @brief Sets an integer shader uniform.
        /// @param name Uniform name.
        /// @param v0 Integer value.
        Common::Return setUniform1i(const Common::String& name, const int& v0);

        /// @brief Sets a floating-point shader uniform.
        /// @param name Uniform name.
        /// @param v0 Floating-point value.
        Common::Return setUniform1f(const Common::String& name, const float& v0);

        /// @brief Sets a 4x4 matrix shader uniform.
        /// @param name Uniform name.
        /// @param mat Matrix value.
        Common::Return setUniformMat4f(const Common::String& name, const Common::Matrix4f& mat);

        /// @brief Sets a three-component vector shader uniform.
        /// @param name Uniform name.
        /// @param vec Vector value.
        Common::Return setUniform3f(const Common::String& name, const Common::Vector3f& vec);

        /// @brief Sets a four-component vector shader uniform.
        /// @param name Uniform name.
        /// @param vec Vector value.
        Common::Return setUniform4f(const Common::String& name, const Common::Vector4f& vec);

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