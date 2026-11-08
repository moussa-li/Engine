#include "Command/LocateElementsCmd.hpp"

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/MeshData.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    LocateElementsCmd::LocateElementsCmd(const LocateElementsParam& params)
        : CommandBase<LocateElementsParam>(params)
    {
    }

    Common::Return LocateElementsCmd::execImpl()
    {
        auto node = Context::instance().getDataNode(getParams().dataNodeId);
        auto* meshData = dynamic_cast<MeshData*>(node.get());
        if (meshData == nullptr)
        {
            LOG(WARNING) << "LocateElementsCmd::exec() could not find mesh data node "
                         << getParams().dataNodeId;
            return Common::Return::Failed;
        }

        auto mesh = meshData->getMesh();
        if (!mesh)
        {
            LOG(WARNING) << "LocateElementsCmd::exec() found a mesh data node without a mesh";
            return Common::Return::Failed;
        }

        Common::DynamicArray<uint32_t> elementIds;
        elementIds.reserve(getParams().count);
        for (uint32_t index = 0; index < getParams().count; ++index)
        {
            elementIds.pushBack(getParams().elementIds[index]);
        }

        RenderWork::instance().locateElements(mesh, elementIds);
        LOG(INFO) << "LocateElementsCmd::exec() queued locate for " << elementIds.size()
                  << " element(s)";
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
