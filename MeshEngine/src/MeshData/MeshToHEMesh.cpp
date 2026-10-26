#include "MeshEngine/MeshData/MeshToHEMesh.hpp"

#include "MeshEngine/Algorithm/ExtractSurface.hpp"
#include "MeshEngine/MeshData/HEMesh.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::ME
{
    namespace
    {
        bool isSupportedElement(ElemType type, int dimension)
        {
            if (dimension == 2)
            {
                return type == ElemType::Tri3 || type == ElemType::Quad4;
            }
            return dimension == 3 && type == ElemType::Tet4;
        }
    } // namespace

    Common::Return MeshToHEMesh::convert(const Common::SharedPtr<Mesh>& mesh,
                                         Common::SharedPtr<HEMesh>& halfEdgeMesh)
    {
        halfEdgeMesh = nullptr;
        if (!mesh || mesh->getNodeNumber() == 0 || mesh->getElemNumber() == 0)
        {
            return Common::Return::BadInput;
        }

        MeshIterator validationIterator(*mesh);
        const int dimension = getTypeDim(validationIterator.currentElem().getType());
        if (dimension != 2 && dimension != 3)
        {
            return Common::Return::BadInput;
        }
        for (; validationIterator.hasMoreElem(); validationIterator.nextElem())
        {
            if (!isSupportedElement(validationIterator.currentElem().getType(), dimension))
            {
                return Common::Return::BadInput;
            }
        }

        auto result = Common::makeShared<HEMesh>();
        MeshIterator nodeIterator(*mesh);
        for (; nodeIterator.hasMoreNode(); nodeIterator.nextNode())
        {
            auto& node = nodeIterator.currentNode();
            if (result->createVertex(node.getId(), static_cast<float>(node.x()),
                                     static_cast<float>(node.y()),
                                     static_cast<float>(node.z())) != Common::Return::Succeed)
            {
                return Common::Return::BadInput;
            }
        }

        if (dimension == 2)
        {
            MeshIterator elementIterator(*mesh);
            for (; elementIterator.hasMoreNode(); elementIterator.nextNode())
            {
                const auto& nodes = elementIterator.currentElem().getNodes();
                if (result->createFace(nodes) != Common::Return::Succeed)
                {
                    return Common::Return::BadInput;
                }
            }
        }
        else
        {
            ExtractSurface surfaceExtractor(mesh);
            const auto surfaceFaces = surfaceExtractor.getSurface();
            if (surfaceFaces.empty())
            {
                return Common::Return::BadInput;
            }

            for (const auto& surfaceFace : surfaceFaces)
            {
                auto& element = mesh->getElemById(surfaceFace.first);
                ElemFaceGetter faceGetter(element);
                const auto faceNodeCount = faceGetter.elemFaceNodeNumber(surfaceFace.second);
                Common::DynamicArray<IdType> faceNodes;
                faceNodes.reserve(static_cast<size_t>(faceNodeCount));
                for (int node = 0; node < faceNodeCount; ++node)
                {
                    auto nodeId = faceGetter.elemFace(surfaceFace.second, node);
                    faceNodes.pushBack(nodeId);
                }
                if (result->createFace(faceNodes) != Common::Return::Succeed)
                {
                    return Common::Return::BadInput;
                }
            }
        }

        halfEdgeMesh = result;
        return Common::Return::Succeed;
    }
} // namespace EgLab::ME