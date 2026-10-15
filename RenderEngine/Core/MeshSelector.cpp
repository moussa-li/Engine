#include "RenderEngine/Core/MeshSelector.hpp"

#include "RenderEngine/Core/RenderFace.hpp"

namespace EgLab::RE
{
    Common::DynamicArray<IdxType> MeshSelector::getHighlightIndices(
        Common::SharedPtr<RenderPrimitive>& primitive, IdxType pickedIndex) const
    {
        Common::DynamicArray<IdxType> highlightIndices;
        auto face = Common::dynamicSharedPtrCast<RenderFace>(primitive);
        if (!face || pickedIndex >= face->getFaceLocations().size())
        {
            highlightIndices.pushBack(pickedIndex);
            return highlightIndices;
        }

        const auto& pickedLocation = face->getFaceLocations()[pickedIndex];
        const auto& faceLocations = face->getFaceLocations();
        for (IdxType index = 0; index < faceLocations.size(); ++index)
        {
            const auto& location = faceLocations[index];
            if (location.elemId == pickedLocation.elemId &&
                location.faceIdx == pickedLocation.faceIdx)
            {
                highlightIndices.pushBack(index);
            }
        }
        return highlightIndices;
    }
} // namespace EgLab::RE