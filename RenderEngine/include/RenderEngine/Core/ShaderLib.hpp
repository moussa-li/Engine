/**
 * @file Global.hpp
 * @author Moussa-Li
 * @brief the global assets common storage place
 * @date 2025-10-25
 */
#pragma once

#include <cstddef>

#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "RenderEngine/Core/Shader.hpp"

#define RegisterShaderLib(NAME, RegisterFunc)              \
    if constexpr (shaderId == NAME)                        \
    {                                                      \
        return ShaderLib::instance().RegisterFunc(buffer); \
    }

namespace EgLab::Common
{
    class String;

} // namespace EgLab::Common
namespace EgLab::RE
{
    /// Built-in shader programs available from ShaderLib.
    enum class ShaderId
    {
        Basic,
        Node,
        Line,
        Easy,
        Highlight,
    };

    /// Loads and provides shared built-in shader programs.
    class RenderEngineAPI ShaderLib : public Common::Singleton<ShaderLib>
    {
    public:
        /// @brief Loads the basic shader source into a buffer.
        /// @param buffer Receives the shader source.
        Common::Return getBasicShader(Common::String &buffer);

        /// @brief Loads the lighting shader source into a buffer.
        /// @param buffer Receives the shader source.
        Common::Return getLightShader(Common::String &buffer);

        /// @brief Loads the node shader source into a buffer.
        /// @param buffer Receives the shader source.
        Common::Return getNodeShader(Common::String &buffer);

        /// @brief Loads shader source by identifier.
        /// @details The identifier selects which built-in shader source is loaded.
        /// @param buffer Receives the shader source.
        Common::Return getShader(ShaderId, Common::String &buffer);

        /// @brief Creates or retrieves the basic shader program.
        /// @details The output reference receives the shader object.
        Common::Return getBasicShader(Common::SharedPtr<Shader> &);

        /// @brief Creates or retrieves the node shader program.
        /// @details The output reference receives the shader object.
        Common::Return getNodeShader(Common::SharedPtr<Shader> &);

        /// @brief Creates or retrieves the line shader program.
        /// @details The output reference receives the shader object.
        Common::Return getLineShader(Common::SharedPtr<Shader> &);

        /// @brief Creates or retrieves the face shader program.
        /// @details The output reference receives the shader object.
        Common::Return getFaceShader(Common::SharedPtr<Shader> &);

        /// @brief Creates or retrieves the highlight shader program.
        /// @details The output reference receives the shader object.
        Common::Return getHighlightShader(Common::SharedPtr<Shader> &);

    private:
        ShaderLib();
        ~ShaderLib();
        friend class Singleton<ShaderLib>;

        class Impl;
        Impl *_data;
    };

    template <ShaderId shaderId>
    Common::Return getShader(Common::String &buffer)
    {
        RegisterShaderLib(ShaderId::Basic, getBasicShader);
        RegisterShaderLib(ShaderId::Node, getNodeShader);
        return Common::Return::Succeed;
    }

} // namespace EgLab::RE