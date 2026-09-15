/**
 * @file Global.hpp
 * @author Moussa-Li
 * @brief the global assets common storage place
 * @date 2025-10-25
 */
#pragma once

#include <cstddef>

#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "Core/RenderEngineAPI.hpp"
#include "RenderEngine/Core/Shader.hpp"

#define RegisterShaderLib(NAME, RegisterFunc)              \
    if constexpr (shaderId == NAME)                        \
    {                                                      \
        return ShaderLib::instance().RegisterFunc(buffer); \
    }

namespace EgLab::Common
{

    enum class Return;
    class String;

} // namespace EgLab::Common
namespace EgLab::RE
{
    enum class ShaderId
    {
        Basic,
        Node,
        Line,
        Easy
    };

    class RenderEngineAPI ShaderLib : public Common::Singleton<ShaderLib>
    {
    public:
        Common::Return getBasicShader(Common::String &buffer);

        Common::Return getLightShader(Common::String &buffer);

        Common::Return getNodeShader(Common::String &buffer);

        Common::Return getShader(ShaderId, Common::String &buffer);

        Common::Return getBasicShader(Common::SharedPtr<Shader> &);

        Common::Return getNodeShader(Common::SharedPtr<Shader> &);

        Common::Return getLineShader(Common::SharedPtr<Shader> &);

        Common::Return getFaceShader(Common::SharedPtr<Shader> &);

    private:
        ShaderLib();
        ~ShaderLib();
        friend class Singleton<ShaderLib>;

        class Impl;
        Impl *_data;
    };

    template <ShaderId shaderId>
    Common::Return ShaderLib(Common::String &buffer)
    {
        RegisterShaderLib(ShaderId::Basic, getBasicShader);
        RegisterShaderLib(ShaderId::Node, getNodeShader);
    }

} // namespace EgLab::RE