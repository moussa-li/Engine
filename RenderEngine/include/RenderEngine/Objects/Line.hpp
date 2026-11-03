#pragma once

#include "Common/Return.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class RenderLine;
    /// Scene entity that renders a line from a sequence of coordinates.
    class RenderEngineAPI Line : public Entity
    {
    public:
        /// @brief Creates a line entity from vertex coordinates.
        /// @param coords Coordinates defining the line vertices.
        Line(Common::DynamicArray<CoordType>& coords);
        /// @brief Releases the line entity resources.
        virtual ~Line();

    private:
    };

} // namespace EgLab::RE