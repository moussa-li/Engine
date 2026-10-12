#include "DataBase/Context.hpp"

#include "DataBase/MeshData.hpp"

namespace EgLab::Platform
{
    Context::Context()
    {
    }

    Context::~Context()
    {
    }

    Common::Return Context::addMeshData(const Common::SharedPtr<ME::Mesh>& mesh,
                                        const Common::String& name)
    {
        std::lock_guard<std::mutex> lock(_dataTreeMutex);
        if (!mesh)
        {
            return Common::Return::BadInput;
        }

        auto* meshData = new MeshData(name, mesh);
        const Common::Return result = _dataTree.addTree(meshData);
        if (result != Common::Return::Succeed)
        {
            delete meshData;
        }
        return result;
    }

    Common::SharedPtr<ME::Mesh> Context::removeMeshData(const ME::Mesh* mesh)
    {
        std::lock_guard<std::mutex> lock(_dataTreeMutex);
        if (mesh == nullptr)
        {
            return nullptr;
        }

        for (DataTree* child : _dataTree.getChildrens())
        {
            auto* meshData = dynamic_cast<MeshData*>(child);
            if (meshData == nullptr || meshData->getMesh().get() != mesh)
            {
                continue;
            }

            Common::SharedPtr<ME::Mesh> removedMesh = meshData->getMesh();
            if (_dataTree.removeTree(child) == Common::Return::Succeed)
            {
                return removedMesh;
            }
            return nullptr;
        }
        return nullptr;
    }
} // namespace EgLab::Platform
