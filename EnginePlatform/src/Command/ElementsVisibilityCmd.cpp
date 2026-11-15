#include "Command/ElementsVisibilityCmd.hpp"

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/MeshData.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    ElementsVisibilityCmd::ElementsVisibilityCmd(const ElementsVisibilityParam& params)
        : CommandBase<ElementsVisibilityParam>(params)
    {
    }

    ElementsVisibilityCmd::~ElementsVisibilityCmd()
    {
        if (getParams().elementIds != nullptr)
        {
            delete[] getParams().elementIds;
        }
    }

    Common::Return ElementsVisibilityCmd::execImpl()
    {
        auto node = Context::instance().getDataNode(getParams().dataNodeId);
        auto* meshData = dynamic_cast<MeshData*>(node.get());
        if (meshData == nullptr)
        {
            LOG(WARNING) << "ElementsVisibilityCmd::exec() could not find mesh data node "
                         << getParams().dataNodeId;
            return Common::Return::Failed;
        }

        auto mesh = meshData->getMesh();
        if (!mesh)
        {
            LOG(WARNING)
                << "ElementsVisibilityCmd::exec() found a mesh data node without a mesh";
            return Common::Return::Failed;
        }

        Common::DynamicArray<uint32_t> elementIds;
        elementIds.reserve(getParams().count);
        for (uint32_t index = 0; index < getParams().count; ++index)
        {
            elementIds.pushBack(getParams().elementIds[index]);
        }

        RenderWork::instance().setElementsVisibility(mesh, getParams().mode, elementIds);
        LOG(INFO) << "ElementsVisibilityCmd::exec() queued visibility change for "
                  << elementIds.size() << " element(s), mode=" << getParams().mode;
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
