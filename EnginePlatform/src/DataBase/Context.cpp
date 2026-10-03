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
} // namespace EgLab::Platform
