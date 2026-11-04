#include "DataBase/MeshData.hpp"

#include <mutex>

namespace EgLab::Platform
{
    MeshData::MeshData(DataNodeId id, const Common::String& name,
                       const Common::SharedPtr<ME::Mesh>& mesh)
        : DataNode(id, name), _mesh(mesh)
    {
    }

    Common::SharedPtr<ME::Mesh> MeshData::getMesh() const
    {
        std::lock_guard<std::mutex> lock(_meshMutex);
        return _mesh;
    }

    void MeshData::setMesh(const Common::SharedPtr<ME::Mesh>& mesh)
    {
        std::lock_guard<std::mutex> lock(_meshMutex);
        _mesh = mesh;
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