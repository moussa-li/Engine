#pragma once

#include "Core/Entity.hpp"
#include "Core/RenderEngineAPI.hpp"

namespace EgLab
{
    class RenderEngineAPI Box : public Entity
    {
        virtual void draw() const override;
    };

} // namespace EgLab