#pragma once

#include "Common/Return.hpp"
#include "Core/Entity.hpp"
#include "Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class RenderLine;
    class RenderEngineAPI Line : public Entity
    {
    public:
        Line(Common::DynamicArray<CoordType>& coords);
        virtual ~Line();

    private:
    };

} // namespace EgLab::RE