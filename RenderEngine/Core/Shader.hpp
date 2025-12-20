#pragma once
#include <unordered_map>

#include "Common/String.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"


namespace EgLab
{
    class String;
    enum class AssetId;
    enum class Return;

    struct ShaderProgramSource
    {
        String VertexSource = "";
        String FragmentSource = "";
        String ComputeSource = "";
    };

    using RendererId = unsigned int;

    class RenderEngineAPI Shader
    {
    public:
        Shader() = default;
        Shader(const AssetId);
        ~Shader();

        Return bind() const;
        Return unBind() const;

        Return setUniform1i(const AssetId& name, int v0);

    private:
        RendererId createShader(const ShaderProgramSource&);

        Return parseShader(ShaderProgramSource&);

    private:
        AssetId _shader;
        RendererId _rendererId;
        std::unordered_map<String, int> _uniformLocationCache;
    };

} // namespace EgLab