#include "MeshEngine/MeshData/MeshVisibility.hpp"

#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::ME
{
    void MeshVisibility::collectNodeIdsHiddenByElements(
        Mesh& mesh, const Common::HashSet<IdType>& hiddenElementIds,
        Common::HashSet<IdType>& hiddenNodeIds)
    {
        if (hiddenElementIds.empty())
        {
            return;
        }

        Common::HashSet<IdType> visibleNodeIds;
        Common::HashSet<IdType> referencedNodeIds;
        MeshIterator elemIterator(mesh);
        for (; elemIterator.hasMoreElem(); elemIterator.nextElem())
        {
            const auto& elem = elemIterator.currentElem();
            const auto& elemNodes = elem.getNodes();
            const bool visible = hiddenElementIds.find(elem.getId()) == hiddenElementIds.end();
            for (size_t index = 0; index < elemNodes.size(); ++index)
            {
                referencedNodeIds.insert(elemNodes[index]);
                if (visible)
                {
                    visibleNodeIds.insert(elemNodes[index]);
                }
            }
        }

        for (auto it = referencedNodeIds.begin(); it.hasNext(); it.next())
        {
            if (visibleNodeIds.find(it.data()) == visibleNodeIds.end())
            {
                hiddenNodeIds.insert(it.data());
            }
        }
    }
} // namespace EgLab::ME
