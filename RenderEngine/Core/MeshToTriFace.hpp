#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/UniquePtr.hpp"
#include "MeshEngine/MeshData/Element.hpp"
#include "RenderEngine/Core/Definites.hpp"

namespace EgLab::ME
{
    class Mesh;
}

namespace EgLab::RE
{
    class MeshToTriFace
    {
    public:
        MeshToTriFace();
        ~MeshToTriFace();

        void setMesh(Common::SharedPtr<ME::Mesh>&);

        Common::DynamicArray<IdxType> getBoundaryFaceIdx() const;

        Common::DynamicArray<IdxType> getBoundaryLineIdx() const;

        Common::DynamicArray<ME::ElemFaceLocation> getBoundaryFaceLocations() const;

        Common::DynamicArray<ME::ElemFaceEdgeLocation> getBoundaryLineLocations() const;

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };
} // namespace EgLab::RE