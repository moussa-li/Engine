#pragma once
#include "Common/SharedPtr.hpp"

namespace EgLab
{
    class Shader;
    class Transform;
    class RenderPrimitive
    {
    public:
        virtual ~RenderPrimitive() = default;
        virtual void setup() = 0;
        virtual void draw(SharedPtr<Shader> shader) = 0;
    };
} // namespace EgLab