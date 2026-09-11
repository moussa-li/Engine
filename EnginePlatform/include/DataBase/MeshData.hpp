#pragma once
#include "Common/SharedPtr.hpp"
#include "DataBase/DataTree.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"


namespace EgLab::Platform
{
    class MeshData : public DataTree
    {
    private:
        Common::SharedPtr<ME::Mesh> _mesh;
    };
} // namespace EgLab::Platform