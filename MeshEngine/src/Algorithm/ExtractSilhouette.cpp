#include "MeshEngine/Algorithm/ExtractSilhouette.hpp"

#include <map>
#include <utility>

#include "MeshEngine/Algorithm/ExtractSurface.hpp"

namespace EgLab::ME
{
    namespace
    {
        struct EdgeFacing
        {
            uint32_t frontFaces{0};
            uint32_t backFaces{0};
        };
    } // namespace

    ExtractSilhouette::ExtractSilhouette(Common::SharedPtr<Mesh> mesh) : _mesh(Common::move(mesh))
    {
    }

    Common::DynamicArray<Common::Pair<IdType, IdType>> ExtractSilhouette::getSilhouetteEdges(
        const CoordType& cameraPosition) const
    {
        if (!_mesh || _mesh->getNodeNumber() == 0 || _mesh->getElemNumber() == 0)
        {
            return {};
        }

        std::map<IdType, CoordType> nodeCoordinates;
        MeshIterator meshIterator(*_mesh);
        for (; meshIterator.hasMoreNode(); meshIterator.nextNode())
        {
            auto& node = meshIterator.currentNode();
            nodeCoordinates.emplace(node.getId(), CoordType(node.x(), node.y(), node.z()));
        }

        std::map<std::pair<IdType, IdType>, EdgeFacing> edgeFacing;
        ExtractSurface surfaceExtractor(_mesh);
        const auto surfaceFaces = surfaceExtractor.getSurface();
        for (const auto& surfaceFace : surfaceFaces)
        {
            auto& element = _mesh->getElemById(surfaceFace.first);
            ElemFaceGetter faceGetter(element);
            const int faceNodeCount = faceGetter.elemFaceNodeNumber(surfaceFace.second);
            if (faceNodeCount < 3)
            {
                continue;
            }

            Common::DynamicArray<IdType> faceNodeIds;
            Common::DynamicArray<CoordType> facePoints;
            CoordType faceCenter(0.0, 0.0, 0.0);
            bool validFace = true;
            for (int node = 0; node < faceNodeCount; ++node)
            {
                const IdType nodeId = faceGetter.elemFace(surfaceFace.second, node);
                const auto nodeCoordinate = nodeCoordinates.find(nodeId);
                if (nodeCoordinate == nodeCoordinates.end())
                {
                    validFace = false;
                    break;
                }
                faceNodeIds.pushBack(nodeId);
                facePoints.pushBack(nodeCoordinate->second);
                faceCenter = faceCenter + nodeCoordinate->second;
            }
            if (!validFace || facePoints.size() < 3)
            {
                continue;
            }
            faceCenter = faceCenter * (1.0 / static_cast<double>(facePoints.size()));

            CoordType normal = (facePoints[1] - facePoints[0]).cross(facePoints[2] - facePoints[0]);
            if (normal.length2() < 1e-12)
            {
                continue;
            }
            normal.normalize();

            CoordType elementCenter(0.0, 0.0, 0.0);
            uint32_t validElementNodeCount = 0;
            for (IdType nodeId : element.getNodes())
            {
                const auto nodeCoordinate = nodeCoordinates.find(nodeId);
                if (nodeCoordinate != nodeCoordinates.end())
                {
                    elementCenter = elementCenter + nodeCoordinate->second;
                    ++validElementNodeCount;
                }
            }
            if (validElementNodeCount > 0)
            {
                elementCenter = elementCenter * (1.0 / static_cast<double>(validElementNodeCount));
                if (normal.dot(faceCenter - elementCenter) < 0.0)
                {
                    normal = normal * -1.0;
                }
            }

            const bool frontFacing = normal.dot(cameraPosition - faceCenter) >= -1e-6;
            for (size_t edge = 0; edge < faceNodeIds.size(); ++edge)
            {
                IdType firstNode = faceNodeIds[edge];
                IdType secondNode = faceNodeIds[(edge + 1) % faceNodeIds.size()];
                if (secondNode < firstNode)
                {
                    std::swap(firstNode, secondNode);
                }
                auto& facing = edgeFacing[{firstNode, secondNode}];
                if (frontFacing)
                {
                    ++facing.frontFaces;
                }
                else
                {
                    ++facing.backFaces;
                }
            }
        }

        Common::DynamicArray<Common::Pair<IdType, IdType>> edges;
        for (const auto& edge : edgeFacing)
        {
            const auto& facing = edge.second;
            const bool silhouette =
                facing.frontFaces > 0 &&
                (facing.backFaces > 0 || facing.frontFaces + facing.backFaces == 1);
            if (silhouette)
            {
                edges.pushBack({edge.first.first, edge.first.second});
            }
        }
        return edges;
    }
} // namespace EgLab::ME