#pragma once

#include "Common/Return.hpp"
#include "Core/Entity.hpp"
#include "Core/RenderEngineAPI.hpp"

namespace EgLab
{
    class RenderNode;
    class RenderEngineAPI Vertex : public Entity
    {
    public:
        Vertex(CoordType &);
        virtual ~Vertex();

        virtual void draw() const override;

    private:
        UniquePtr<RenderNode> _node;
    };
} // namespace EgLab
