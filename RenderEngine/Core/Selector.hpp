#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class RenderPrimitive;

    class RenderEngineAPI Selector
    {
    public:
        virtual ~Selector() = default;
        virtual Common::DynamicArray<IdxType> getHighlightIndices(
            Common::SharedPtr<RenderPrimitive>& primitive, IdxType pickedIndex) const = 0;
    };
} // namespace EgLab::RE