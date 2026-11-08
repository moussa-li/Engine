#include "Command/HighlightCmd.hpp"

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/MeshData.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    HighlightCmd::HighlightCmd(const HighlightParam& params) : CommandBase<HighlightParam>(params)
    {
    }

    Common::Return HighlightCmd::execImpl()
    {
        Common::DynamicArray<Common::SharedPtr<ME::Mesh>> meshes;
        meshes.reserve(getParams().count);

        for (uint32_t index = 0; index < getParams().count; ++index)
        {
            const DataNodeId nodeId = getParams().dataNodeIds[index];
            auto node = Context::instance().getDataNode(nodeId);
            auto* meshData = dynamic_cast<MeshData*>(node.get());
            if (meshData == nullptr)
            {
                LOG(WARNING) << "HighlightCmd::exec() skipped unknown data node " << nodeId;
                continue;
            }

            auto mesh = meshData->getMesh();
            if (mesh)
            {
                meshes.pushBack(mesh);
            }
        }

        RenderWork::instance().setHighlightedMeshes(meshes);
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
