#include <cstddef>
#include <cstdlib>

#include <gtest/gtest.h>

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "GeometryEngine/Common/GeometryImportReport.hpp"
#include "GeometryEngine/Geometry/Geometry.hpp"
#include "GeometryEngine/IO/GeometryImporter.hpp"
#include "GeometryEngine/Kernel/GeometryKernelRegistry.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "GeometryEngine/Topology/Topology.hpp"

namespace
{
    using namespace EgLab;

    TEST(GeometryTopology, createsAndLinksEntities)
    {
        GE::Topology topology;

        const auto solid = topology.createNode(GE::TopologyType::Solid, Common::String("Body"));
        const auto face = topology.createNode(GE::TopologyType::Face, Common::String("Face_1"));
        const auto edge = topology.createNode(GE::TopologyType::Edge, Common::String("Edge_1"));
        const auto vertex = topology.createNode(GE::TopologyType::Vertex, Common::String("Vertex_1"));

        ASSERT_NE(solid, GE::INVALID_IDX);
        ASSERT_NE(face, GE::INVALID_IDX);
        ASSERT_NE(edge, GE::INVALID_IDX);
        ASSERT_NE(vertex, GE::INVALID_IDX);

        EXPECT_TRUE(topology.link(GE::TopologyType::Solid, solid, GE::TopologyType::Face, face) ==
                    Common::Return::Succeed);
        EXPECT_TRUE(topology.link(GE::TopologyType::Face, face, GE::TopologyType::Edge, edge) ==
                    Common::Return::Succeed);
        EXPECT_TRUE(topology.link(GE::TopologyType::Edge, edge, GE::TopologyType::Vertex, vertex) ==
                    Common::Return::Succeed);

        // A solid may only own faces.
        EXPECT_TRUE(topology.link(GE::TopologyType::Solid, solid, GE::TopologyType::Edge, edge) ==
                    Common::Return::BadInput);

        const GE::TopologyNode* solidNode = topology.getNode(GE::TopologyType::Solid, solid);
        const GE::TopologyNode* faceNode = topology.getNode(GE::TopologyType::Face, face);
        const GE::TopologyNode* edgeNode = topology.getNode(GE::TopologyType::Edge, edge);
        const GE::TopologyNode* vertexNode = topology.getNode(GE::TopologyType::Vertex, vertex);
        ASSERT_NE(solidNode, nullptr);
        ASSERT_NE(faceNode, nullptr);
        ASSERT_NE(edgeNode, nullptr);
        ASSERT_NE(vertexNode, nullptr);

        EXPECT_EQ(solidNode->getChildren().size(), 1u);
        EXPECT_EQ(solidNode->getChildren()[0], face);
        EXPECT_TRUE(solidNode->getName() == Common::String("Body"));

        EXPECT_EQ(faceNode->getParents().size(), 1u);
        EXPECT_EQ(faceNode->getParents()[0], solid);
        EXPECT_EQ(faceNode->getChildren().size(), 1u);
        EXPECT_EQ(faceNode->getChildren()[0], edge);

        EXPECT_EQ(edgeNode->getParents().size(), 1u);
        EXPECT_EQ(edgeNode->getParents()[0], face);
        EXPECT_EQ(edgeNode->getChildren()[0], vertex);

        EXPECT_EQ(vertexNode->getParents().size(), 1u);
        EXPECT_EQ(vertexNode->getParents()[0], edge);

        EXPECT_EQ(topology.getNodeCount(GE::TopologyType::Face), 1u);
        EXPECT_EQ(topology.findNodeIndex(GE::TopologyType::Face, faceNode->getId()), face);
        EXPECT_EQ(topology.findNodeIndex(GE::TopologyType::Solid, 9999u), GE::INVALID_IDX);
    }

    TEST(GeometryTessellation, appendsWithIndexOffset)
    {
        GE::Tessellation first;
        const auto a = first.addNode(GE::Vec3(0.0, 0.0, 0.0));
        const auto b = first.addNode(GE::Vec3(1.0, 0.0, 0.0));
        const auto c = first.addNode(GE::Vec3(0.0, 1.0, 0.0));
        first.addTriangle(a, b, c);

        GE::Tessellation second;
        const auto d = second.addNode(GE::Vec3(1.0, 1.0, 0.0));
        const auto e = second.addNode(GE::Vec3(2.0, 1.0, 0.0));
        const auto f = second.addNode(GE::Vec3(1.0, 2.0, 0.0));
        second.addTriangle(d, e, f);

        first.append(second);

        EXPECT_EQ(first.getNodeCount(), 6u);
        EXPECT_EQ(first.getElemCount(), 2u);
        ASSERT_EQ(first.getElemNodeIndices().size(), 6u);
        EXPECT_EQ(first.getElemNodeIndices()[0], 0u);
        EXPECT_EQ(first.getElemNodeIndices()[3], 3u);
        EXPECT_EQ(first.getElemNodeIndices()[4], 4u);
        EXPECT_EQ(first.getElemNodeIndices()[5], 5u);
        EXPECT_EQ(first.getElemSizes()[0], 3u);
        EXPECT_EQ(first.getElemSizes()[1], 3u);
    }

    TEST(GeometryDocument, exposesFeatureMesh)
    {
        GE::GeometryDocument document;
        const auto faceIndex = document.createFeature(GE::TopologyType::Face, Common::String("Face_A"));
        ASSERT_NE(faceIndex, GE::INVALID_IDX);

        GE::Geometry* geometry = document.getGeometry(GE::TopologyType::Face, faceIndex);
        ASSERT_NE(geometry, nullptr);
        EXPECT_TRUE(geometry->isValid());

        const auto node0 = geometry->getTessellation().addNode(GE::Vec3(0.0, 0.0, 0.0));
        const auto node1 = geometry->getTessellation().addNode(GE::Vec3(1.0, 0.0, 0.0));
        const auto node2 = geometry->getTessellation().addNode(GE::Vec3(0.0, 1.0, 0.0));
        geometry->getTessellation().addTriangle(node0, node1, node2);

        const GE::GeometryFeature feature = document.getFeature(GE::TopologyType::Face, faceIndex);
        ASSERT_TRUE(feature.isValid());
        EXPECT_EQ(feature.getId(), document.getTopology().getNode(GE::TopologyType::Face, faceIndex)->getId());
        EXPECT_EQ(feature.getMesh().getNodeCount(), 3u);
        EXPECT_EQ(feature.getMesh().getElemCount(), 1u);
        ASSERT_EQ(feature.getMesh().getElemNodeIndices().size(), 3u);
        EXPECT_EQ(feature.getMesh().getElemNodeIndices()[2], 2u);

        const GE::GeometryFeature missing = document.getFeature(GE::TopologyType::Face, 42u);
        EXPECT_FALSE(missing.isValid());
        EXPECT_TRUE(missing.getMesh().empty());
    }

    TEST(GeometryKernel, registryAlwaysProvidesAdapter)
    {
        GE::GeometryKernelRegistry& registry = GE::GeometryKernelRegistry::instance();

        bool hasNull = false;
        const Common::DynamicArray<Common::String> names = registry.getKernelNames();
        for (std::size_t i = 0; i < names.size(); ++i)
        {
            if (names[i] == Common::String("Null")) hasNull = true;
        }
        EXPECT_TRUE(hasNull);

        Common::UniquePtr<GE::IGeometryImporter> importer = registry.createImporter(
            Common::String("KernelThatDoesNotExist"), GE::GeometryImportOptions());
        ASSERT_NE(importer.get(), nullptr);
        EXPECT_GT(importer->getName().size(), 0u);

#ifdef GEOMETRY_KERNEL_OCCT
        EXPECT_NE(registry.findFactory(Common::String("OCCT")), nullptr);
        EXPECT_TRUE(GE::GeometryKernelRegistry::getDefaultKernelName() == Common::String("OCCT"));
#else
        EXPECT_EQ(registry.findFactory(Common::String("OCCT")), nullptr);
        EXPECT_TRUE(GE::GeometryKernelRegistry::getDefaultKernelName() == Common::String("Null"));
#endif
    }

    TEST(GeometryImporter, reportsMissingFile)
    {
        GE::GeometryDocument document;
        GE::GeometryImporter importer;

        const GE::GeometryImportReport report =
            importer.importFile(Common::String("this_file_does_not_exist.stp"), document);

        EXPECT_FALSE(report.isSucceed());
        EXPECT_EQ(document.getFeatureCount(GE::TopologyType::Face), 0u);
        EXPECT_EQ(document.getFeatureCount(GE::TopologyType::Solid), 0u);
        EXPECT_TRUE(document.getLastReport().getStatus() == report.getStatus());
    }

#ifdef GEOMETRY_KERNEL_OCCT
    TEST(GeometryImporter, importsGeometryFileFromEnvironment)
    {
        const char* path = std::getenv("EGLAB_GEOMETRY_TEST_STEP");
        if (path == nullptr || path[0] == '\0')
        {
            GTEST_SKIP() << "set EGLAB_GEOMETRY_TEST_STEP to a STEP or BREP file to run this test";
        }

        GE::GeometryDocument document;
        GE::GeometryImporter importer(Common::String("OCCT"));
        const GE::GeometryImportReport report = importer.importFile(Common::String(path), document);

        ASSERT_TRUE(report.isSucceed()) << report.getMessage().c_str();
        EXPECT_GT(report.getTriangleCount(), 0u);

        const std::size_t faceCount = document.getFeatureCount(GE::TopologyType::Face);
        EXPECT_GT(faceCount, 0u);
        for (GE::IdxType i = 0; i < faceCount; ++i)
        {
            const GE::GeometryFeature face = document.getFeature(GE::TopologyType::Face, i);
            ASSERT_TRUE(face.isValid());
            EXPECT_GT(face.getMesh().getNodeCount(), 0u);
        }

        const std::size_t solidCount = document.getFeatureCount(GE::TopologyType::Solid);
        for (GE::IdxType i = 0; i < solidCount; ++i)
        {
            const GE::GeometryFeature solid = document.getFeature(GE::TopologyType::Solid, i);
            ASSERT_TRUE(solid.isValid());
            EXPECT_FALSE(solid.getChildren().empty());
            EXPECT_FALSE(solid.getMesh().empty());

            for (std::size_t child = 0; child < solid.getChildren().size(); ++child)
            {
                const GE::GeometryFeature face =
                    document.getFeature(GE::TopologyType::Face, solid.getChildren()[child]);
                ASSERT_TRUE(face.isValid());
                EXPECT_GE(face.getGeometry().getArea(), 0.0);
                EXPECT_FALSE(face.getParents().empty());
            }
        }
    }
#endif

} // namespace
