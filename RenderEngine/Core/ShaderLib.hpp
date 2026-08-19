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

    class String;

    class RenderEngineAPI ShaderLib : public Common::Singleton<ShaderLib>
    {
    public:
        Common::Return getBasicShader(String &buffer);

        Common::Return getLightShader(String &buffer);

        Common::Return getNodeShader(String &buffer);

        Common::Return getShader(ShaderId, String &buffer);

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
    Common::Return ShaderLib(String &buffer)
    {
        RegisterShaderLib(ShaderId::Basic, getBasicShader);
        RegisterShaderLib(ShaderId::Node, getNodeShader);
    }

} // namespace EgLab::RE