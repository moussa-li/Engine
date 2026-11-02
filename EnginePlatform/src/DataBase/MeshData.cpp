#include "DataBase/MeshData.hpp"

namespace EgLab::Platform
{
    MeshData::MeshData(DataNodeId id, const Common::String& name,
                       const Common::SharedPtr<ME::Mesh>& mesh)
        : DataNode(id, Common::String("MeshData: ") + name), _mesh(mesh)
    {
    }

    Common::SharedPtr<ME::Mesh>& MeshData::getMesh()
    {
        return _mesh;
    }

    const Common::SharedPtr<ME::Mesh>& MeshData::getMesh() const
    {
        return _mesh;
    }

    const Common::Vector4f& MeshData::getColor() const
    {
        return _color;
    }

    void MeshData::setColor(const Common::Vector4f& color)
    {
        _color = color;
    }
} // namespace EgLab::Platform