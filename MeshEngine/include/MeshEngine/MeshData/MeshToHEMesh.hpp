#pragma once

#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"

namespace EgLab::ME
{
    class HEMesh;
    class Mesh;

    class MeshToHEMesh
    {
    public:
        static Common::Return convert(const Common::SharedPtr<Mesh>& mesh,
                                      Common::SharedPtr<HEMesh>& halfEdgeMesh);
    };
} // namespace EgLab::ME