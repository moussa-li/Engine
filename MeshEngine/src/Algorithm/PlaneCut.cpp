#include "MeshEngine/Algorithm/PlaneCut.hpp"

#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::ME
{
    PlaneCut::PlaneCut(Common::SharedPtr<Mesh> mesh) : _mesh(Common::move(mesh))
    {
    }

    PlaneCutResult PlaneCut::cut(const CoordType& normal, const CoordType& point) const
    {
        PlaneCutResult result;
        if (!_mesh || _mesh->getElemNumber() == 0)
        {
            return result;
        }

        CoordType unitNormal = normal;
        const Scalar normalLength = unitNormal.length();
        if (normalLength < 1e-12)
        {
            return result;
        }
        unitNormal = unitNormal * (1.0 / normalLength);

        MeshIterator iterator(*_mesh);
        for (; iterator.hasMoreElem(); iterator.nextElem())
        {
            const auto& element = iterator.currentElem();
            const auto& nodeIds = element.getNodes();
            if (nodeIds.empty())
            {
                continue;
            }

            CoordType centroid(0.0, 0.0, 0.0);
            for (size_t nodeIndex = 0; nodeIndex < nodeIds.size(); ++nodeIndex)
            {
                const auto& node = _mesh->getNodeById(nodeIds[nodeIndex]);
                centroid = centroid + CoordType(node.x(), node.y(), node.z());
            }
            centroid = centroid * (1.0 / static_cast<Scalar>(nodeIds.size()));

            const Scalar signedDistance = unitNormal.dot(centroid - point);
            if (signedDistance >= 0.0)
            {
                result.positiveElementIds.pushBack(element.getId());
            }
            else
            {
                result.negativeElementIds.pushBack(element.getId());
            }
        }
        return result;
    }
} // namespace EgLab::ME
