#pragma once

#include "Common/Return.hpp"
#include "Core/Entity.hpp"
#include "Core/RenderEngineAPI.hpp"

namespace EgLab
{
    class RenderEngineAPI Box : public Entity
    {
    public:
        Box(Vector3f position, Vector3f rotation, Vector3f scale);

    private:
        Return createMesh();

        // virtual void draw() const override;
    };

} // namespace EgLab