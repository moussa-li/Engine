#pragma once

#include "RenderEngine/Core/Property.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    /// Stores configurable rendering and depth-test settings.
    class RenderEngineAPI RenderConfigure
    {
    public:
        /// Enables or disables depth testing.
        Property<bool> enableDeepTest{true};
        /// Comparison function used by the depth test.
        enum class DeepTestType
        {
            /// Never pass the depth test.
            NEVER,
            /// Pass when the incoming depth is less than the stored depth.
            LESS,
            /// Pass when the incoming depth equals the stored depth.
            EQUAL,
            /// Pass when the incoming depth is less than or equal to the stored depth.
            LEQUAL,
            /// Pass when the incoming depth is greater than the stored depth.
            GREATER,
            /// Pass when the incoming depth is not equal to the stored depth.
            NOTEQUAL,
            /// Pass when the incoming depth is greater than or equal to the stored depth.
            GEQUAL,
            /// Always pass the depth test.
            ALWAYS
        };
        /// Depth comparison function, defaulting to LESS.
        Property<DeepTestType> deepTestType{DeepTestType::LESS};

        /// @brief Creates render configuration with default settings.
        RenderConfigure() = default;
        /// @brief Destroys the render configuration.
        ~RenderConfigure() = default;
    };

} // namespace EgLab::RE