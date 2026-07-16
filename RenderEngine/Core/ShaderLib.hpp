/**
 * @file Global.hpp
 * @author Moussa-Li
 * @brief the global assets common storage place
 * @date 2025-10-25
 */
#pragma once

#include <cstddef>

#include "Common/Singleton.hpp"
#include "Core/RenderEngineAPI.hpp"

#define RegisterShaderLib(NAME, RegisterFunc)              \
    if constexpr (shaderId == NAME)                        \
    {                                                      \
        return ShaderLib::instance().RegisterFunc(buffer); \
    }

namespace EgLab
{
    enum class ShaderId
    {
        Basic,
        Easy
    };

    class String;
    enum class Return;

    class RenderEngineAPI ShaderLib : public Singleton<ShaderLib>
    {
    public:
        Return getBasicShader(String &buffer);

        Return getLightShader(String &buffer);

    private:
        ShaderLib();
        ~ShaderLib();
        friend class Singleton<ShaderLib>;

        class Impl;
        Impl *_data;
    };

    template <ShaderId shaderId>
    Return ShaderLib(String &buffer)
    {
        RegisterShaderLib(ShaderId::Basic, getBasicShader);
    }

} // namespace EgLab