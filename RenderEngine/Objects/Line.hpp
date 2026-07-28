#pragma once

#include "Common/Return.hpp"
#include "Core/Entity.hpp"
#include "Core/RenderEngineAPI.hpp"

namespace EgLab
{
    class RenderLine;
    class RenderEngineAPI Line : public Entity
    {
    public:
        Line(DynamicArray<CoordType>& coords);
        virtual ~Line();

        virtual void draw() const override;

    private:
        UniquePtr<RenderLine> _line;
    };

} // namespace EgLab