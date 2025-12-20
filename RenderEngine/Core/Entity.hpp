#pragma once

#include "Definites.hpp"

namespace EgLab
{
    class Entity
    {
    public:
        virtual void draw() const;

        virtual void update(DeltaTime time);
    };

} // namespace EgLab