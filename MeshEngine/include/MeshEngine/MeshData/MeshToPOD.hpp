#pragma once

#include "Common/SharedPtr.hpp"
#include "MeshEngine/MeshData/MeshPOD.hpp"

namespace EgLab::ME
{
    class Mesh;

    class MeshToPOD
    {
    public:
        static MeshPOD serialize(const Common::SharedPtr<Mesh> mesh);
        static MeshPOD convert(const Common::SharedPtr<Mesh> mesh);
        static Common::SharedPtr<Mesh> deserialize(const MeshPOD &pod);
    };
} // namespace EgLab::ME