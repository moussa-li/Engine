#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Pair.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/UniquePtr.hpp"
#include "MeshEngine/Common/Definitions.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::ME
{
    class Mesh;
    class ExtractSurface
    {
    public:
        ExtractSurface(Common::SharedPtr<Mesh>);
        ~ExtractSurface();

        // Replaces the set of elements excluded from surface extraction. Elements whose ids
        // are in hiddenElementIds are ignored, so faces shared between a visible and a hidden
        // element become boundary faces. Pass an empty list to extract the full surface.
        void setHiddenElementIds(const Common::DynamicArray<IdType>& hiddenElementIds);

        Common::DynamicArray<Common::Pair<IdType, IdxType>> getSurface();

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };

} // namespace EgLab::ME