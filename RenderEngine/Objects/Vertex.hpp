#pragma once

#include "Common/Return.hpp"
#include "Core/Entity.hpp"
#include "Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class RenderNode;
    class RenderEngineAPI Vertex : public Entity
    {
    public:
        Vertex(CoordType &);
        virtual ~Vertex();
    };
} // namespace EgLab::RE
