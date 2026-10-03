#pragma once
#include "Common/SharedPtr.hpp"
#include "DataBase/DataTree.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"


namespace EgLab::Platform
{
    class MeshData : public DataTree
    {
    public:
        MeshData(const Common::String& name, const Common::SharedPtr<ME::Mesh>& mesh);

        const Common::SharedPtr<ME::Mesh>& getMesh() const;

    private:
        Common::SharedPtr<ME::Mesh> _mesh;
    };
} // namespace EgLab::Platform