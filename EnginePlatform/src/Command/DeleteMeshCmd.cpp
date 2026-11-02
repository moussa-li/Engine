#include "Command/DeleteMeshCmd.hpp"

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/MeshData.hpp"
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
        auto& context = Context::instance();
        auto node = context.getDataNode(getParams().dataNodeId);
        auto* meshData = dynamic_cast<MeshData*>(node.get());
        if (meshData == nullptr)
        {
            LOG(WARNING) << "DeleteMeshCmd::exec() could not find mesh data node";
            return Common::Return::Failed;
        }

        auto mesh = meshData->getMesh();
        auto removedNode = context.removeDataNode(getParams().dataNodeId);
        if (!removedNode)
        {
            return Common::Return::Failed;
        }
        if (mesh)
        {
            RenderWork::instance().removeMesh(mesh);
        }
        LOG(INFO) << "DeleteMeshCmd::exec() removed mesh data and queued render cleanup";
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform