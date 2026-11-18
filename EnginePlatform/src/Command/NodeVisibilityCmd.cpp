#include "Command/NodeVisibilityCmd.hpp"

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/MeshData.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    NodeVisibilityCmd::NodeVisibilityCmd(const NodeVisibilityParam& params)
        : CommandBase<NodeVisibilityParam>(params)
    {
    }

    NodeVisibilityCmd::~NodeVisibilityCmd()
    {
        if (getParams().nodeIds != nullptr)
        {
            delete[] getParams().nodeIds;
        }
    }

    Common::Return NodeVisibilityCmd::execImpl()
    {
        auto node = Context::instance().getDataNode(getParams().dataNodeId);
        auto* meshData = dynamic_cast<MeshData*>(node.get());
        if (meshData == nullptr)
        {
            LOG(WARNING) << "NodeVisibilityCmd::exec() could not find mesh data node "
                         << getParams().dataNodeId;
            return Common::Return::Failed;
        }

        auto mesh = meshData->getMesh();
        if (!mesh)
        {
            LOG(WARNING) << "NodeVisibilityCmd::exec() found a mesh data node without a mesh";
            return Common::Return::Failed;
        }

        Common::DynamicArray<uint32_t> nodeIds;
        nodeIds.reserve(getParams().count);
        for (uint32_t index = 0; index < getParams().count; ++index)
        {
            nodeIds.pushBack(getParams().nodeIds[index]);
        }

        RenderWork::instance().setNodeVisibility(mesh, getParams().mode, nodeIds);
        LOG(INFO) << "NodeVisibilityCmd::exec() queued visibility change for " << nodeIds.size()
                  << " node(s), mode=" << getParams().mode;
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
