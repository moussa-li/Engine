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
    void GeometryMeshBuild::clear()
    {
        mesh = nullptr;
        elementFeatures.clear();
        nodeFeatures.clear();
        edgeSegmentIndices.clear();
        edgeSegmentFeatures.clear();
        triangleCount = 0;
        vertexCount = 0;
        edgeCount = 0;
        edgeSegmentCount = 0;
    }

    namespace
    {
        /// Appends the tessellation nodes to the mesh and returns the id of the first one.
        ME::IdType appendNodes(const GE::Tessellation& tessellation, ME::Mesh& mesh,
                               ME::IdType& nextNodeId)
        {
            const auto& nodes = tessellation.getNodes();
            const ME::IdType base = nextNodeId;
            for (std::size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
            {
                ME::Node node;
                node.setId(nextNodeId);
                ME::CoordType coord(nodes[nodeIndex][0], nodes[nodeIndex][1], nodes[nodeIndex][2]);
                node.setXYZ(coord);
                mesh.addNode(Common::move(node));
                ++nextNodeId;
            }
            return base;
        }
    } // namespace

    Common::Return GeometryMeshConverter::convert(
        const Common::SharedPtr<GE::GeometryDocument>& document, GeometryMeshBuild& outBuild)
    {
        outBuild.clear();

        if (!document)
        {
            return Common::Return::BadInput;
        }

        Common::SharedPtr<ME::Mesh> mesh = Common::makeShared<ME::Mesh>();
        Common::DynamicArray<GeometryFeatureRef>& nodeFeatures = outBuild.nodeFeatures;

        ME::IdType nextNodeId = 1;
        ME::IdType nextElemId = 1;

        // Keeps the node -> feature table addressable by node index (node id - 1).
        const auto syncNodeFeatures = [&nodeFeatures](ME::IdType nextNodeId) {
            const std::size_t nodeCount = static_cast<std::size_t>(nextNodeId - 1);
            while (nodeFeatures.size() < nodeCount)
            {
                nodeFeatures.pushBack(GeometryFeatureRef());
            }
        };

        // ---- faces: Tri3 elements ------------------------------------------------------
        const std::size_t faceCount = document->getFeatureCount(GE::TopologyType::Face);
        for (std::size_t faceIndex = 0; faceIndex < faceCount; ++faceIndex)
        {
            const GE::Geometry* geometry =
                document->getGeometry(GE::TopologyType::Face, static_cast<GE::IdxType>(faceIndex));
            if (geometry == nullptr)
            {
                continue;
            }

            const GE::Tessellation& tessellation = geometry->getTessellation();
            const auto& elemNodeIndices = tessellation.getElemNodeIndices();
            const auto& elemSizes = tessellation.getElemSizes();

            const ME::IdType faceNodeBase = appendNodes(tessellation, *mesh, nextNodeId);
            syncNodeFeatures(nextNodeId);

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
                outBuild.elementFeatures.pushBack(ref);

                ++nextElemId;
                ++outBuild.triangleCount;
                cursor += nodeCount;
            }
        }

        if (outBuild.triangleCount == 0)
        {
            outBuild.clear();
            return Common::Return::Failed;
        }

        // ---- edges: polyline segments -------------------------------------------------
        const std::size_t edgeCount = document->getFeatureCount(GE::TopologyType::Edge);
        for (std::size_t edgeIndex = 0; edgeIndex < edgeCount; ++edgeIndex)
        {
            const GE::Geometry* geometry =
                document->getGeometry(GE::TopologyType::Edge, static_cast<GE::IdxType>(edgeIndex));
            if (geometry == nullptr)
            {
                continue;
            }

            const GE::Tessellation& tessellation = geometry->getTessellation();
            const auto& nodes = tessellation.getNodes();
            if (nodes.size() < 2)
            {
                continue;
            }

            const ME::IdType edgeNodeBase = appendNodes(tessellation, *mesh, nextNodeId);
            syncNodeFeatures(nextNodeId);

            GeometryFeatureRef ref;
            ref.featureType = static_cast<uint32_t>(GE::TopologyType::Edge);
            ref.featureIndex = static_cast<uint32_t>(edgeIndex);

            // Consecutive polyline pairs become pickable segments, laid out contiguously per
            // edge so a feature lookup is one range of segments.
            for (std::size_t node = 0; node + 1 < nodes.size(); ++node)
            {
                outBuild.edgeSegmentIndices.pushBack(static_cast<uint32_t>(edgeNodeBase + node));
                outBuild.edgeSegmentIndices.pushBack(
                    static_cast<uint32_t>(edgeNodeBase + node + 1));
                outBuild.edgeSegmentFeatures.pushBack(ref);
                ++outBuild.edgeSegmentCount;
            }
            ++outBuild.edgeCount;
        }

        // ---- vertices: single nodes ---------------------------------------------------
        const std::size_t vertexCount = document->getFeatureCount(GE::TopologyType::Vertex);
        for (std::size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
        {
            const GE::Geometry* geometry = document->getGeometry(
                GE::TopologyType::Vertex, static_cast<GE::IdxType>(vertexIndex));
            if (geometry == nullptr)
            {
                continue;
            }

            const GE::Tessellation& tessellation = geometry->getTessellation();
            if (tessellation.getNodes().empty())
            {
                continue;
            }

            const ME::IdType vertexNodeBase = appendNodes(tessellation, *mesh, nextNodeId);
            syncNodeFeatures(nextNodeId);

            GeometryFeatureRef ref;
            ref.featureType = static_cast<uint32_t>(GE::TopologyType::Vertex);
            ref.featureIndex = static_cast<uint32_t>(vertexIndex);
            nodeFeatures[static_cast<std::size_t>(vertexNodeBase - 1)] = ref;
            ++outBuild.vertexCount;
        }

        outBuild.mesh = Common::move(mesh);
        return Common::Return::Succeed;
    }

} // namespace EgLab::Platform
