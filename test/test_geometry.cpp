#include <cstddef>
#include <cstdlib>

#include <chrono>
#include <fstream>

#include <gtest/gtest.h>

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "GeometryEngine/Common/GeometryImportReport.hpp"
#include "GeometryEngine/Geometry/Geometry.hpp"
#include "GeometryEngine/IO/GeometryImporter.hpp"
#include "GeometryEngine/Kernel/GeometryKernelRegistry.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "GeometryEngine/Model/GeometryElementMap.hpp"
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

    TEST(GeometryDocument, renamesFeature)
    {
        GE::GeometryDocument document;
        const GE::IdxType face =
            document.createFeature(GE::TopologyType::Face, Common::String("Face_0"));
        ASSERT_NE(face, GE::INVALID_IDX);

        ASSERT_TRUE(document.setFeatureName(GE::TopologyType::Face, face,
                                            Common::String("TopFace")) ==
                    Common::Return::Succeed);
        EXPECT_EQ(document.getFeature(GE::TopologyType::Face, face).getName(),
                  Common::String("TopFace"));

        EXPECT_TRUE(document.setFeatureName(GE::TopologyType::Face, face + 1,
                                            Common::String("Missing")) ==
                    Common::Return::BadInput);
    }

    TEST(GeometryImporter, rejectsEmptyBuffer)
    {
        GE::GeometryDocument document;
        GE::GeometryImporter importer;

        const GE::GeometryImportReport report = importer.importBuffer(nullptr, 0, document);

        EXPECT_FALSE(report.isSucceed());
        EXPECT_EQ(document.getFeatureCount(GE::TopologyType::Face), 0u);
        EXPECT_TRUE(document.getLastReport().getStatus() == report.getStatus());
    }

    TEST(GeometryImporter, rejectsGarbageBuffer)
    {
        const char garbage[] = "not a serialized brep";
        GE::GeometryDocument document;
        GE::GeometryImporter importer;

        const GE::GeometryImportReport report =
            importer.importBuffer(garbage, sizeof(garbage) - 1, document);

        EXPECT_FALSE(report.isSucceed());
        EXPECT_EQ(document.getFeatureCount(GE::TopologyType::Face), 0u);
    }

#ifdef GEOMETRY_KERNEL_OCCT
    TEST(GeometryImporter, importsSerializedBrepBufferFromEnvironment)
    {
        const char* path = std::getenv("EGLAB_GEOMETRY_TEST_BREP");
        if (path == nullptr || path[0] == '\0')
        {
            GTEST_SKIP() << "set EGLAB_GEOMETRY_TEST_BREP to a BREP file to run this test";
        }

        std::ifstream file(path, std::ios::binary);
        ASSERT_TRUE(file.good());
        file.seekg(0, std::ios::end);
        const std::streamoff fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        ASSERT_GT(fileSize, 0);

        Common::DynamicArray<char> bytes;
        bytes.resize(static_cast<std::size_t>(fileSize));
        ASSERT_TRUE(file.read(bytes.data(), static_cast<std::streamsize>(fileSize)).good());

        GE::GeometryDocument document;
        GE::GeometryImporter importer(Common::String("OCCT"));
        const GE::GeometryImportReport report =
            importer.importBuffer(bytes.data(), bytes.size(), document);

        ASSERT_TRUE(report.isSucceed()) << report.getMessage().c_str();
        EXPECT_GT(document.getFeatureCount(GE::TopologyType::Face), 0u);
        EXPECT_GT(report.getTriangleCount(), 0u);
    }
#endif

    TEST(GeometryElementMap, resolvesElementsToFeaturesAndBack)
    {
        GE::GeometryDocument document;
        const auto solid = document.createFeature(GE::TopologyType::Solid, Common::String("Body"));
        const auto face0 = document.createFeature(GE::TopologyType::Face, Common::String("Face_0"));
        const auto face1 = document.createFeature(GE::TopologyType::Face, Common::String("Face_1"));
        ASSERT_NE(solid, GE::INVALID_IDX);
        ASSERT_NE(face0, GE::INVALID_IDX);
        ASSERT_NE(face1, GE::INVALID_IDX);
        ASSERT_TRUE(document.linkFeatures(GE::TopologyType::Solid, solid, GE::TopologyType::Face,
                                          face0) == Common::Return::Succeed);
        ASSERT_TRUE(document.linkFeatures(GE::TopologyType::Solid, solid, GE::TopologyType::Face,
                                          face1) == Common::Return::Succeed);

        // Two triangles for Face_0, one for Face_1.
        Common::DynamicArray<GE::FeatureRef> elementFeatures;
        elementFeatures.pushBack(GE::FeatureRef(GE::TopologyType::Face, face0));
        elementFeatures.pushBack(GE::FeatureRef(GE::TopologyType::Face, face0));
        elementFeatures.pushBack(GE::FeatureRef(GE::TopologyType::Face, face1));

        GE::GeometryElementMap map;
        ASSERT_TRUE(map.build(document, elementFeatures) == Common::Return::Succeed);
        EXPECT_EQ(map.getElementCount(), 3u);

        // Element ids are 1-based, id 0 and ids past the table stay unresolved.
        EXPECT_TRUE(map.getElementFeature(1) == elementFeatures[0]);
        EXPECT_TRUE(map.getElementFeature(3) == elementFeatures[2]);
        EXPECT_FALSE(map.getElementFeature(0).isValid());
        EXPECT_FALSE(map.getElementFeature(4).isValid());

        Common::DynamicArray<GE::IdType> elements;
        map.collectElements(GE::FeatureRef(GE::TopologyType::Face, face0), elements);
        ASSERT_EQ(elements.size(), 2u);
        EXPECT_EQ(elements[0], 1u);
        EXPECT_EQ(elements[1], 2u);

        // An aggregate feature contributes the elements of the faces it contains.
        elements.clear();
        map.collectElements(GE::FeatureRef(GE::TopologyType::Solid, solid), elements);
        ASSERT_EQ(elements.size(), 3u);
        EXPECT_EQ(elements[0], 1u);
        EXPECT_EQ(elements[1], 2u);
        EXPECT_EQ(elements[2], 3u);

        elements.clear();
        map.collectElements(GE::FeatureRef(GE::TopologyType::Face, 7), elements);
        EXPECT_TRUE(elements.empty());
    }

    TEST(GeometryElementMap, keepsSlotsStableWhenElementFeaturesInterleave)
    {
        GE::GeometryDocument document;
        const auto face0 = document.createFeature(GE::TopologyType::Face, Common::String("Face_0"));
        const auto face1 = document.createFeature(GE::TopologyType::Face, Common::String("Face_1"));
        const auto face2 = document.createFeature(GE::TopologyType::Face, Common::String("Face_2"));
        ASSERT_NE(face0, GE::INVALID_IDX);
        ASSERT_NE(face1, GE::INVALID_IDX);
        ASSERT_NE(face2, GE::INVALID_IDX);

        // An importer that emits triangles out of feature order must still resolve every
        // feature to one slot, exactly like a table grouped per feature.
        Common::DynamicArray<GE::FeatureRef> elementFeatures;
        elementFeatures.pushBack(GE::FeatureRef(GE::TopologyType::Face, face1));
        elementFeatures.pushBack(GE::FeatureRef(GE::TopologyType::Face, face0));
        elementFeatures.pushBack(GE::FeatureRef(GE::TopologyType::Face, face1));
        elementFeatures.pushBack(GE::FeatureRef(GE::TopologyType::Face, face2));
        elementFeatures.pushBack(GE::FeatureRef(GE::TopologyType::Face, face0));

        GE::GeometryElementMap map;
        ASSERT_TRUE(map.build(document, elementFeatures) == Common::Return::Succeed);

        Common::DynamicArray<GE::IdType> elements;
        map.collectElements(GE::FeatureRef(GE::TopologyType::Face, face0), elements);
        ASSERT_EQ(elements.size(), 2u);
        EXPECT_EQ(elements[0], 2u);
        EXPECT_EQ(elements[1], 5u);

        elements.clear();
        map.collectElements(GE::FeatureRef(GE::TopologyType::Face, face1), elements);
        ASSERT_EQ(elements.size(), 2u);
        EXPECT_EQ(elements[0], 1u);
        EXPECT_EQ(elements[1], 3u);

        elements.clear();
        map.collectElements(GE::FeatureRef(GE::TopologyType::Face, face2), elements);
        ASSERT_EQ(elements.size(), 1u);
        EXPECT_EQ(elements[0], 4u);

        // Unknown indices must stay unresolved instead of reusing a stale slot.
        elements.clear();
        map.collectElements(GE::FeatureRef(GE::TopologyType::Face, 99), elements);
        EXPECT_TRUE(elements.empty());

        elements.clear();
        map.collectElements(GE::FeatureRef(GE::TopologyType::Edge, face0), elements);
        EXPECT_TRUE(elements.empty());
    }

    TEST(GeometryElementMap, buildStaysFastForLargeInterleavedTables)
    {
        // Box selection and import both build the map, then resolve one feature per hit
        // element. The slot lookup used to scan every feature, which made large interleaved
        // tables quadratic.
        const std::size_t featureCount = 5000;
        const std::size_t repeats = 10;

        GE::GeometryDocument document;
        for (std::size_t feature = 0; feature < featureCount; ++feature)
        {
            ASSERT_NE(document.createFeature(GE::TopologyType::Face, Common::String("Face")),
                      GE::INVALID_IDX);
        }

        Common::DynamicArray<GE::FeatureRef> elementFeatures;
        elementFeatures.reserve(featureCount * repeats);
        for (std::size_t repeat = 0; repeat < repeats; ++repeat)
        {
            for (std::size_t feature = 0; feature < featureCount; ++feature)
            {
                elementFeatures.pushBack(
                    GE::FeatureRef(GE::TopologyType::Face, static_cast<GE::IdxType>(feature)));
            }
        }

        GE::GeometryElementMap map;
        const auto start = std::chrono::steady_clock::now();
        ASSERT_TRUE(map.build(document, elementFeatures) == Common::Return::Succeed);
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::steady_clock::now() - start)
                                   .count();

        EXPECT_EQ(map.getElementCount(), featureCount * repeats);
        EXPECT_LT(elapsedMs, 500) << "slot lookup must not scan every feature";

        Common::DynamicArray<GE::IdType> elements;
        map.collectElements(GE::FeatureRef(GE::TopologyType::Face, 0), elements);
        ASSERT_EQ(elements.size(), repeats);
        EXPECT_EQ(elements[0], 1u);
        EXPECT_EQ(elements[1], 1u + featureCount);
    }

} // namespace
