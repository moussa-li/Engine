#include "DataBase/MeshData.hpp"

namespace EgLab::Platform
{
    MeshData::MeshData(const Common::String& name, const Common::SharedPtr<ME::Mesh>& mesh)
        : DataTree(Common::String("MeshData: ") + name), _mesh(mesh)
    {
    }

    const Common::SharedPtr<ME::Mesh>& MeshData::getMesh() const
    {
        return _mesh;
    }
} // namespace EgLab::Platform