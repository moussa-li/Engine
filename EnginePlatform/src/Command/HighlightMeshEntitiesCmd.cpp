#include "Command/HighlightMeshEntitiesCmd.hpp"

#include "Common/HashSet.hpp"
#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/MeshData.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    HighlightMeshEntitiesCmd::HighlightMeshEntitiesCmd(const MeshEntityHighlightParam& params,
                                                       bool focus)
        : CommandBase<MeshEntityHighlightParam>(params), _focus(focus)
    {
    }

    Common::Return HighlightMeshEntitiesCmd::execImpl()
    {
        const MeshEntityHighlightParam& params = getParams();
        if (params.nodeCount == 0 && params.elementCount == 0)
        {
            if (_focus)
            {
                RenderWork::instance().clearMeshEntityFocus();
            }
            else
            {
                RenderWork::instance().clearMeshEntityHighlight();
            }
            return Common::Return::Succeed;
        }

        auto node = Context::instance().getDataNode(params.dataNodeId);
        auto* meshData = dynamic_cast<MeshData*>(node.get());
        if (meshData == nullptr)
        {
            LOG(WARNING) << getCmdName().c_str() << "::exec() could not find mesh data node "
                         << params.dataNodeId;
            return Common::Return::Failed;
        }

        const auto mesh = meshData->getMesh();
        if (!mesh)
        {
            LOG(WARNING) << getCmdName().c_str()
                         << "::exec() found a mesh data node without a mesh";
            return Common::Return::Failed;
        }

        // The render side addresses nodes by their 0-based render index, so the ids are
        // resolved by walking the mesh once instead of assuming id == index + 1.
        Common::DynamicArray<uint32_t> nodeIndices;
        if (params.nodeCount > 0)
        {
            Common::HashSet<ME::IdType> requestedNodes;
            for (uint32_t index = 0; index < params.nodeCount; ++index)
            {
                requestedNodes.insert(static_cast<ME::IdType>(params.nodeIds[index]));
            }

            uint32_t renderIndex = 0;
            ME::MeshIterator nodeIterator(*mesh);
            for (; nodeIterator.hasMoreNode(); nodeIterator.nextNode(), ++renderIndex)
            {
                if (requestedNodes.find(nodeIterator.currentNode().getId()) != requestedNodes.end())
                {
                    nodeIndices.pushBack(renderIndex);
                }
            }
        }

        Common::DynamicArray<uint32_t> elementIds;
        elementIds.reserve(params.elementCount);
        for (uint32_t index = 0; index < params.elementCount; ++index)
        {
            elementIds.pushBack(params.elementIds[index]);
        }

        if (_focus)
        {
            RenderWork::instance().setMeshEntityFocus(mesh, elementIds, nodeIndices);
        }
        else
        {
            RenderWork::instance().setMeshEntityHighlight(mesh, elementIds, nodeIndices);
        }
        LOG(INFO) << getCmdName().c_str() << "::exec() highlighted " << nodeIndices.size()
                  << " node(s) and " << elementIds.size() << " element(s)";
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
