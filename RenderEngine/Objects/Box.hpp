#pragma once

#include "Common/Return.hpp"
#include "Core/Entity.hpp"
#include "Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class RenderEngineAPI Box : public Entity
    {
    public:
        Box();

    private:
        Common::Return createMesh();

        // virtual void draw() const override;
    };

} // namespace EgLab::RE