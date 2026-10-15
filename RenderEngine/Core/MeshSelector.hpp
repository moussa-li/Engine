#pragma once

#include "RenderEngine/Core/Selector.hpp"

namespace EgLab::RE
{
    class RenderEngineAPI MeshSelector : public Selector
    {
    public:
        Common::DynamicArray<IdxType> getHighlightIndices(
            Common::SharedPtr<RenderPrimitive>& primitive, IdxType pickedIndex) const override;
    };
} // namespace EgLab::RE