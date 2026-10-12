#include "Command/DeleteMeshCmd.hpp"

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    DeleteMeshCmd::DeleteMeshCmd(const DeleteMeshParam& params)
        : CommandBase<DeleteMeshParam>(params)
    {
    }

    Common::Return DeleteMeshCmd::execImpl()
    {
        auto* mesh = reinterpret_cast<ME::Mesh*>(getParams().meshAddress);
        auto removedMesh = Context::instance().removeMeshData(mesh);
        if (!removedMesh)
        {
            LOG(WARNING) << "DeleteMeshCmd::exec() could not find mesh data";
            return Common::Return::Failed;
        }

        RenderWork::instance().removeMesh(removedMesh);
        LOG(INFO) << "DeleteMeshCmd::exec() removed mesh data and queued render cleanup";
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform