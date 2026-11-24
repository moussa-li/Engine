#include <cstddef>

#include <gtest/gtest.h>

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "DataBase/GeometryMeshConverter.hpp"
#include "GeometryEngine/Common/GeometryTypes.hpp"
#include "GeometryEngine/Geometry/Geometry.hpp"
#include "GeometryEngine/Geometry/Tessellation.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"
#include "MeshEngine/MeshData/Element.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace
{
    using namespace EgLab;

    /// Builds a minimal B-Rep document: one triangular face, one 3-point edge, one vertex.
    void buildDocument(GE::GeometryDocument& document)
    {
        const auto face = document.createFeature(GE::TopologyType::Face, Common::String("Face_1"));
        const auto edge = document.createFeature(GE::TopologyType::Edge, Common::String("Edge_1"));
        const auto vertex =
            document.createFeature(GE::TopologyType::Vertex, Common::String("Vertex_1"));
        ASSERT_NE(face, GE::INVALID_IDX);
        ASSERT_NE(edge, GE::INVALID_IDX);
        ASSERT_NE(vertex, GE::INVALID_IDX);

        GE::Geometry* faceGeometry = document.getGeometry(GE::TopologyType::Face, face);
        ASSERT_NE(faceGeometry, nullptr);
        GE::Tessellation& faceMesh = faceGeometry->getTessellation();
        faceMesh.addNode(GE::Vec3(0.0, 0.0, 0.0));
        faceMesh.addNode(GE::Vec3(1.0, 0.0, 0.0));
        faceMesh.addNode(GE::Vec3(0.0, 1.0, 0.0));
        faceMesh.addTriangle(0, 1, 2);

        GE::Geometry* edgeGeometry = document.getGeometry(GE::TopologyType::Edge, edge);
        ASSERT_NE(edgeGeometry, nullptr);
        GE::Tessellation& edgeMesh = edgeGeometry->getTessellation();
        edgeMesh.addNode(GE::Vec3(0.0, 0.0, 0.0));
        edgeMesh.addNode(GE::Vec3(0.5, 0.0, 0.0));
        edgeMesh.addNode(GE::Vec3(1.0, 0.0, 0.0));
        const GE::IdxType polyline[] = {0, 1, 2};
        edgeMesh.addElement(polyline, 3);

        GE::Geometry* vertexGeometry = document.getGeometry(GE::TopologyType::Vertex, vertex);
        ASSERT_NE(vertexGeometry, nullptr);
        GE::Tessellation& vertexMesh = vertexGeometry->getTessellation();
        const GE::IdxType pointIndex = vertexMesh.addNode(GE::Vec3(0.0, 1.0, 0.0));
        vertexMesh.addElement(&pointIndex, 1);

        ASSERT_TRUE(document.linkFeatures(GE::TopologyType::Face, face, GE::TopologyType::Edge,
                                          edge) == Common::Return::Succeed);
        ASSERT_TRUE(document.linkFeatures(GE::TopologyType::Edge, edge, GE::TopologyType::Vertex,
                                          vertex) == Common::Return::Succeed);
    }

    TEST(GeometryMeshConverter, emitsFaceEdgeAndVertexFeatures)
    {
        auto document = Common::makeShared<GE::GeometryDocument>();
        buildDocument(*document);

        Platform::GeometryMeshBuild build;
        ASSERT_TRUE(Platform::GeometryMeshConverter::convert(document, build) ==
                    Common::Return::Succeed);

        ASSERT_TRUE(build.mesh != nullptr);
        EXPECT_EQ(build.triangleCount, 1u);
        EXPECT_EQ(build.vertexCount, 1u);
        EXPECT_EQ(build.edgeCount, 1u);

        // Face layer: one Tri3 element owning the face feature.
        ASSERT_EQ(build.elementFeatures.size(), 1u);
        EXPECT_EQ(build.elementFeatures[0].featureType,
                  static_cast<uint32_t>(GE::TopologyType::Face));

        // Edge layer: the 3-point polyline becomes two segments of the same edge.
        ASSERT_EQ(build.edgeSegmentCount, 2u);
        ASSERT_EQ(build.edgeSegmentIndices.size(), 4u);
        ASSERT_EQ(build.edgeSegmentFeatures.size(), 2u);
        for (std::size_t segment = 0; segment < build.edgeSegmentFeatures.size(); ++segment)
        {
            EXPECT_EQ(build.edgeSegmentFeatures[segment].featureType,
                      static_cast<uint32_t>(GE::TopologyType::Edge));
        }
        for (std::size_t index = 0; index < build.edgeSegmentIndices.size(); ++index)
        {
            EXPECT_LT(build.edgeSegmentIndices[index], build.mesh->getNodeNumber());
        }
        // Consecutive segments share the middle node of the polyline.
        EXPECT_EQ(build.edgeSegmentIndices[1], build.edgeSegmentIndices[2]);

        // Vertex layer: exactly one node references the vertex feature; face and edge nodes
        // must stay un-referenced so only B-Rep vertices are ever rendered as markers.
        ASSERT_EQ(build.nodeFeatures.size(), build.mesh->getNodeNumber());
        std::size_t vertexNodes = 0;
        std::size_t vertexNodeIndex = build.nodeFeatures.size();
        for (std::size_t index = 0; index < build.nodeFeatures.size(); ++index)
        {
            if (!build.nodeFeatures[index].isValid())
            {
                continue;
            }
            ++vertexNodes;
            vertexNodeIndex = index;
            EXPECT_EQ(build.nodeFeatures[index].featureType,
                      static_cast<uint32_t>(GE::TopologyType::Vertex));
        }
        EXPECT_EQ(vertexNodes, 1u);
        ASSERT_LT(vertexNodeIndex, build.nodeFeatures.size());
        // The first node belongs to the face triangle and the second one to the edge polyline.
        EXPECT_FALSE(build.nodeFeatures[0].isValid());
        EXPECT_FALSE(build.nodeFeatures[1].isValid());
        EXPECT_TRUE(vertexNodeIndex > 2u);

        // Triangles reference the first three nodes only, and every element is a Tri3.
        ASSERT_EQ(build.mesh->getElemNumber(), 1u);
        EXPECT_EQ(build.mesh->getElemById(1).getType(), ME::ElemType::Tri3);
    }

} // namespace
