#pragma once

#include "Common/Return.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class RenderNode;
    /// Scene entity that renders a single vertex.
    class RenderEngineAPI Vertex : public Entity
    {
    public:
        /// @brief Creates a vertex entity at the specified coordinate.
        /// @details The coordinate determines the vertex position.
        Vertex(CoordType &);
        /// @brief Releases the vertex entity resources.
        virtual ~Vertex();
    };
} // namespace EgLab::RE
