#pragma once

#include "Property.hpp"
#include "RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class RenderEngineAPI RenderConfigure
    {
    public:
        Property<bool> enableDeepTest{true};
        enum class DeepTestType
        {
            NEVER,
            LESS,
            EQUAL,
            LEQUAL,
            GREATER,
            NOTEQUAL,
            GEQUAL,
            ALWAYS
        };
        Property<DeepTestType> deepTestType{DeepTestType::LESS};

        RenderConfigure() = default;
        ~RenderConfigure() = default;
    };

} // namespace EgLab::RE