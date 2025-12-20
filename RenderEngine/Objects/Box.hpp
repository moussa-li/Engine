#pragma once

#include "Core/Entity.hpp"

namespace EgLab
{
    class Box : public Entity
    {
        virtual void draw() const override;
    };

} // namespace EgLab