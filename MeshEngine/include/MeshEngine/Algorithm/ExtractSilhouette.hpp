#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Pair.hpp"
#include "Common/SharedPtr.hpp"
#include "MeshEngine/Common/Definitions.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::ME
{
    class ExtractSilhouette
    {
    public:
        explicit ExtractSilhouette(Common::SharedPtr<Mesh> mesh);

        Common::DynamicArray<Common::Pair<IdType, IdType>> getSilhouetteEdges(
            const CoordType& cameraPosition) const;

    private:
        Common::SharedPtr<Mesh> _mesh;
    };
} // namespace EgLab::ME