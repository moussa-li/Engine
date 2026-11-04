#pragma once

#include <mutex>

#include "Common/SharedPtr.hpp"
#include "Common/Vector.hpp"
#include "DataBase/DataNode.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::Platform
{
    class MeshData : public DataNode
    {
    public:
        MeshData(DataNodeId id, const Common::String& name,
                 const Common::SharedPtr<ME::Mesh>& mesh);

        Common::SharedPtr<ME::Mesh> getMesh() const;
        void setMesh(const Common::SharedPtr<ME::Mesh>& mesh);
        const Common::Vector4f& getColor() const;
        void setColor(const Common::Vector4f& color);

    private:
        mutable std::mutex _meshMutex;
        Common::SharedPtr<ME::Mesh> _mesh;
        Common::Vector4f _color{0.8f, 0.8f, 0.8f, 1.0f};
    };
} // namespace EgLab::Platform