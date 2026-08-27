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

        Common::DynamicArray<Common::Pair<IdType, IdxType>> getSurface();

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };

} // namespace EgLab::ME