#include "DataBase/GeometryMeshConverter.hpp"

#include "Common/Log.hpp"
#include "GeometryEngine/Common/GeometryTypes.hpp"
#include "GeometryEngine/Geometry/Tessellation.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "GeometryEngine/Topology/Topology.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"
#include "MeshEngine/MeshData/Element.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/Node.hpp"

namespace EgLab::Platform
{
    Common::Return GeometryMeshConverter::convert(
        const Common::SharedPtr<GE::GeometryDocument>& document, Common::SharedPtr<ME::Mesh>& outMesh,
        Common::DynamicArray<GeometryFeatureRef>& outElementFeatures, uint32_t& outTriangleCount)
    {
        outMesh = nullptr;
        outElementFeatures.clear();
        outTriangleCount = 0;

        if (!document)
        {
            return Common::Return::BadInput;
        }

        const std::size_t faceCount = document->getFeatureCount(GE::TopologyType::Face);

        Common::SharedPtr<ME::Mesh> mesh = Common::makeShared<ME::Mesh>();
        Common::DynamicArray<GeometryFeatureRef> features;

        ME::IdType nextNodeId = 1;
        ME::IdType nextElemId = 1;

        for (std::size_t faceIndex = 0; faceIndex < faceCount; ++faceIndex)
        {
            const GE::Geometry* geometry =
                document->getGeometry(GE::TopologyType::Face, static_cast<GE::IdxType>(faceIndex));
            if (geometry == nullptr)
            {
                continue;
            }

            const GE::Tessellation& tessellation = geometry->getTessellation();
            const auto& nodes = tessellation.getNodes();
            const auto& elemNodeIndices = tessellation.getElemNodeIndices();
            const auto& elemSizes = tessellation.getElemSizes();

            // 该 Face 的所有节点追加到 mesh（Face 间共享边节点重复，D1 已接受）。
            const ME::IdType faceNodeBase = nextNodeId;
            for (std::size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
            {
                ME::Node node;
                node.setId(nextNodeId);
                ME::CoordType coord(nodes[nodeIndex][0], nodes[nodeIndex][1], nodes[nodeIndex][2]);
                node.setXYZ(coord);
                mesh->addNode(Common::move(node));
                ++nextNodeId;
            }

            // 仅转换三角形面，生成 Tri3 单元并记录 elemId -> feature 映射。
            std::size_t cursor = 0;
            for (std::size_t element = 0; element < elemSizes.size(); ++element)
            {
                const std::uint32_t nodeCount = elemSizes[element];
                if (nodeCount != 3)
                {
                    LOG(WARNING) << "GeometryMeshConverter skipped a non-triangle face element"
                                    " (nodes="
                                 << nodeCount << ")";
                    cursor += nodeCount;
                    continue;
                }

                ME::Elem elem;
                elem.setId(nextElemId);
                elem.setType(ME::ElemType::Tri3);
                for (std::uint32_t corner = 0; corner < 3; ++corner)
                {
                    const std::uint32_t localIndex = elemNodeIndices[cursor + corner];
                    elem.setNode(corner, static_cast<ME::IdType>(faceNodeBase + localIndex));
                }
                mesh->addElem(Common::move(elem));

                GeometryFeatureRef ref;
                ref.featureType = static_cast<uint32_t>(GE::TopologyType::Face);
                ref.featureIndex = static_cast<uint32_t>(faceIndex);
                features.pushBack(ref);

                ++nextElemId;
                ++outTriangleCount;
                cursor += nodeCount;
            }
        }

        if (outTriangleCount == 0)
        {
            outMesh = nullptr;
            outElementFeatures.clear();
            return Common::Return::Failed;
        }

        outMesh = Common::move(mesh);
        outElementFeatures = Common::move(features);
        return Common::Return::Succeed;
    }

} // namespace EgLab::Platform
