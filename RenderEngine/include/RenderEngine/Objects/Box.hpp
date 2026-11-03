#pragma once

#include "Common/Return.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    /// Scene entity that creates and renders a box mesh.
    class RenderEngineAPI Box : public Entity
    {
    public:
        /// @brief Creates a box entity and its mesh data.
        Box();

    private:
        Common::Return createMesh();

        // virtual void draw() const override;
    };

} // namespace EgLab::RE