#pragma once
#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class Shader;
    class Transform;
    class RenderEngineAPI RenderPrimitive
    {
    public:
        virtual ~RenderPrimitive();
        virtual void setup() = 0;
        virtual void draw(Common::SharedPtr<Shader> shader) = 0;
    };
} // namespace EgLab::RE