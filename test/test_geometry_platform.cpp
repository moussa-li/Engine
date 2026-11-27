#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

#include <gtest/gtest.h>

#include "Command/GeometryWire.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "DataBase/GeometryMeshConverter.hpp"
#include "GeometryEngine/Common/GeometryImportReport.hpp"
#include "GeometryEngine/Common/GeometryTypes.hpp"
#include "GeometryEngine/Geometry/Geometry.hpp"
#include "GeometryEngine/Geometry/Tessellation.hpp"
#include "GeometryEngine/IO/GeometryImporter.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"
#include "MeshEngine/MeshData/Element.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/Node.hpp"
#include "Work/GeometryZebra.hpp"
#include "Work/GeometryTessellation.hpp"

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
        faceMesh.addNode(GE::Vec3(0.0, 0.0, 0.0), GE::Vec2(2.0, 4.0));
        faceMesh.addNode(GE::Vec3(1.0, 0.0, 0.0), GE::Vec2(6.0, 4.0));
        faceMesh.addNode(GE::Vec3(0.0, 1.0, 0.0), GE::Vec2(2.0, 9.0));
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

    /// Every edge segment must join two consecutive nodes of that edge's own polyline. This
    /// catches node-index vs node-id mix-ups, which shift all segments onto foreign nodes.
    void expectEdgeSegmentsMatchPolylines(const GE::GeometryDocument& document,
                                          const Platform::GeometryMeshBuild& build)
    {
        const std::size_t edgeCount = document.getFeatureCount(GE::TopologyType::Edge);
        std::size_t checkedSegments = 0;
        for (std::size_t edgeIndex = 0; edgeIndex < edgeCount; ++edgeIndex)
        {
            const GE::Geometry* geometry =
                document.getGeometry(GE::TopologyType::Edge, static_cast<GE::IdxType>(edgeIndex));
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

            const GE::FeatureRef ref(GE::TopologyType::Edge, static_cast<GE::IdxType>(edgeIndex));
            std::size_t segmentInEdge = 0;
            for (std::size_t segment = 0; segment < build.edgeSegmentFeatures.size(); ++segment)
            {
                if (build.edgeSegmentFeatures[segment] != ref)
                {
                    continue;
                }
                ASSERT_LT(segmentInEdge + 1, nodes.size())
                    << "edge " << edgeIndex << " has more segments than polyline nodes";
                const std::uint32_t firstIndex = build.edgeSegmentIndices[segment * 2];
                const std::uint32_t secondIndex = build.edgeSegmentIndices[segment * 2 + 1];
                ASSERT_LT(firstIndex, build.mesh->getNodeNumber());
                ASSERT_LT(secondIndex, build.mesh->getNodeNumber());

                const ME::Node& firstNode = build.mesh->getNodeById(firstIndex + 1);
                const ME::Node& secondNode = build.mesh->getNodeById(secondIndex + 1);
                const GE::Vec3& expectedFirst = nodes[segmentInEdge];
                const GE::Vec3& expectedSecond = nodes[segmentInEdge + 1];
                EXPECT_NEAR(firstNode.x(), expectedFirst[0], 1e-9);
                EXPECT_NEAR(firstNode.y(), expectedFirst[1], 1e-9);
                EXPECT_NEAR(firstNode.z(), expectedFirst[2], 1e-9);
                EXPECT_NEAR(secondNode.x(), expectedSecond[0], 1e-9);
                EXPECT_NEAR(secondNode.y(), expectedSecond[1], 1e-9);
                EXPECT_NEAR(secondNode.z(), expectedSecond[2], 1e-9);
                ++segmentInEdge;
                ++checkedSegments;
            }
            EXPECT_EQ(segmentInEdge + 1, nodes.size())
                << "edge " << edgeIndex << " segments do not cover its polyline";
        }
        EXPECT_GT(checkedSegments, 0u);
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

        // Edge layer: the 3-point polyline becomes two segments of the same edge, addressed by
        // 0-based mesh node index (render primitives do not use mesh node ids).
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

        // The segment indices must address the edge nodes, not the face nodes: verify the
        // positions they resolve to are the sampled polyline points.
        for (std::size_t segment = 0; segment < build.edgeSegmentCount; ++segment)
        {
            const std::uint32_t first = build.edgeSegmentIndices[segment * 2];
            const std::uint32_t second = build.edgeSegmentIndices[segment * 2 + 1];
            const ME::Node& firstNode = build.mesh->getNodeById(first + 1);
            const ME::Node& secondNode = build.mesh->getNodeById(second + 1);
            EXPECT_NEAR(firstNode.x(), 0.5 * static_cast<double>(segment), 1e-9);
            EXPECT_NEAR(firstNode.y(), 0.0, 1e-9);
            EXPECT_NEAR(secondNode.x(), 0.5 * static_cast<double>(segment + 1), 1e-9);
            EXPECT_NEAR(secondNode.y(), 0.0, 1e-9);
        }

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

        // Surface UV: one entry per mesh node, face nodes normalised to [0,1] per face, and
        // edge/vertex nodes left at the origin.
        ASSERT_EQ(build.nodeUvs.size(), build.mesh->getNodeNumber());
        EXPECT_NEAR(build.nodeUvs[0][0], 0.0f, 1e-6f);
        EXPECT_NEAR(build.nodeUvs[0][1], 0.0f, 1e-6f);
        EXPECT_NEAR(build.nodeUvs[1][0], 1.0f, 1e-6f);
        EXPECT_NEAR(build.nodeUvs[1][1], 0.0f, 1e-6f);
        EXPECT_NEAR(build.nodeUvs[2][0], 0.0f, 1e-6f);
        EXPECT_NEAR(build.nodeUvs[2][1], 1.0f, 1e-6f);
        for (std::size_t index = 3; index < build.nodeUvs.size(); ++index)
        {
            EXPECT_NEAR(build.nodeUvs[index][0], 0.0f, 1e-6f);
            EXPECT_NEAR(build.nodeUvs[index][1], 0.0f, 1e-6f);
        }

        expectEdgeSegmentsMatchPolylines(*document, build);
    }

    /// Imports the file named by EGLAB_GEOMETRY_TEST_STEP and validates every edge segment
    /// against its edge's tessellated polyline.
    TEST(GeometryMeshConverter, importedEdgeSegmentsMatchPolylines)
    {
        const char* path = std::getenv("EGLAB_GEOMETRY_TEST_STEP");
        if (path == nullptr || path[0] == '\0')
        {
            GTEST_SKIP() << "set EGLAB_GEOMETRY_TEST_STEP to a STEP or BREP file to run this test";
        }

        auto document = Common::makeShared<GE::GeometryDocument>();
        GE::GeometryImporter importer(Common::String("OCCT"));
        const GE::GeometryImportReport report =
            importer.importFile(Common::String(path), *document);
        ASSERT_TRUE(report.isSucceed()) << report.getMessage().c_str();

        Platform::GeometryMeshBuild build;
        ASSERT_TRUE(Platform::GeometryMeshConverter::convert(document, build) ==
                    Common::Return::Succeed);

        EXPECT_GT(build.edgeSegmentCount, 0u);
        EXPECT_EQ(build.edgeSegmentIndices.size(), build.edgeSegmentCount * 2u);
        expectEdgeSegmentsMatchPolylines(*document, build);

        // Vertex markers must never collide with edge or face nodes.
        std::size_t vertexNodes = 0;
        for (std::size_t index = 0; index < build.nodeFeatures.size(); ++index)
        {
            if (build.nodeFeatures[index].isValid())
            {
                ++vertexNodes;
            }
        }
        EXPECT_EQ(vertexNodes, build.vertexCount);
    }

    /// The zebra stripe density arrives from the UI or an IPC client, so it must be normalised
    /// before it reaches the shader.
    TEST(GeometryZebraSettings, clampsFrequencyIntoSupportedRange)
    {
        EXPECT_FLOAT_EQ(Platform::GeometryZebra::clampFrequency(12.0f), 12.0f);
        EXPECT_FLOAT_EQ(Platform::GeometryZebra::clampFrequency(0.5f),
                        Platform::GeometryZebra::MIN_FREQUENCY);
        EXPECT_FLOAT_EQ(Platform::GeometryZebra::clampFrequency(500.0f),
                        Platform::GeometryZebra::MAX_FREQUENCY);

        // Absent, zero, negative and non-finite values must not produce a degenerate pattern.
        EXPECT_FLOAT_EQ(Platform::GeometryZebra::clampFrequency(0.0f),
                        Platform::GeometryZebra::DEFAULT_FREQUENCY);
        EXPECT_FLOAT_EQ(Platform::GeometryZebra::clampFrequency(-4.0f),
                        Platform::GeometryZebra::DEFAULT_FREQUENCY);
        EXPECT_FLOAT_EQ(Platform::GeometryZebra::clampFrequency(
                            std::numeric_limits<float>::quiet_NaN()),
                        Platform::GeometryZebra::DEFAULT_FREQUENCY);
    }

    /// The zebra light direction is a free UI/AI parameter, so a degenerate direction must fall
    /// back to the default instead of collapsing every stripe into one value.
    TEST(GeometryZebraSettings, normalisesLightDirection)
    {
        const Common::Vector3f scaled =
            Platform::GeometryZebra::clampDirection(Common::Vector3f(0.0f, 0.0f, 4.0f));
        EXPECT_FLOAT_EQ(scaled.x(), 0.0f);
        EXPECT_FLOAT_EQ(scaled.y(), 0.0f);
        EXPECT_FLOAT_EQ(scaled.z(), 1.0f);

        const Common::Vector3f fallback =
            Platform::GeometryZebra::clampDirection(Common::Vector3f(0.0f, 0.0f, 0.0f));
        EXPECT_FLOAT_EQ(fallback.z(), 1.0f);

        const Common::Vector3f diagonal =
            Platform::GeometryZebra::clampDirection(Common::Vector3f(1.0f, 1.0f, 0.0f));
        EXPECT_NEAR(diagonal.length(), 1.0f, 1e-6f);
        EXPECT_NEAR(diagonal.x(), diagonal.y(), 1e-6f);
    }

    /// The render settings panel and the SetGeometryTessellation command share this holder, so
    /// it must keep the clamped options that ImportGeometry will consume.
    TEST(GeometryTessellationSettings, storesClampedImportOptions)
    {
        auto& settings = Platform::GeometryTessellationSettings::instance();
        const GE::GeometryImportOptions original = settings.getOptions();

        GE::GeometryImportOptions options;
        options.setLinearDeflection(-5.0);
        options.setAngularDeflection(0.0);
        options.setRelativeDeflection(true);
        options.setParallel(false);
        EXPECT_GT(options.getLinearDeflection(), 0.0);
        EXPECT_GT(options.getAngularDeflection(), 0.0);

        settings.setOptions(options);
        const GE::GeometryImportOptions stored = settings.getOptions();
        EXPECT_DOUBLE_EQ(stored.getLinearDeflection(), options.getLinearDeflection());
        EXPECT_DOUBLE_EQ(stored.getAngularDeflection(), options.getAngularDeflection());
        EXPECT_TRUE(stored.isRelativeDeflection());
        EXPECT_FALSE(stored.isParallel());

        settings.setOptions(original);
    }

    namespace
    {
        Common::DynamicArray<uint8_t> buildUpdateGeometryPayload(
            const char* nodeName, uint64_t dataNodeId, const char* topoData, uint32_t topoSize,
            const Common::DynamicArray<Platform::GeometryFeatureNameEntry>& names)
        {
            Platform::UpdateGeometryWireHeader header{};
            header.magic = Platform::UpdateGeometryWireHeader::WireMagic;
            header.version = Platform::UpdateGeometryWireHeader::WireVersion;
            header.topoDataSize = topoSize;
            header.nameCount = static_cast<uint32_t>(names.size());
            header.dataNodeId = dataNodeId;
            std::snprintf(header.nodeName, sizeof(header.nodeName), "%s", nodeName);

            Common::DynamicArray<uint8_t> payload;
            payload.pushBack(reinterpret_cast<const uint8_t*>(&header), sizeof(header));
            if (topoSize > 0)
            {
                payload.pushBack(reinterpret_cast<const uint8_t*>(topoData), topoSize);
            }
            for (std::size_t index = 0; index < names.size(); ++index)
            {
                payload.pushBack(reinterpret_cast<const uint8_t*>(&names[index]), sizeof(names[index]));
            }
            return payload;
        }

        Platform::GeometryFeatureNameEntry makeNameEntry(uint32_t featureType,
                                                         uint32_t featureIndex,
                                                         const char* name)
        {
            Platform::GeometryFeatureNameEntry entry{};
            entry.featureType = featureType;
            entry.featureIndex = featureIndex;
            std::snprintf(entry.name, sizeof(entry.name), "%s", name);
            return entry;
        }
    } // namespace

    TEST(GeometryUpdateWire, parsesHeaderTopologyAndFeatureNames)
    {
        const char topoData[] = "BREPBYTES";
        Common::DynamicArray<Platform::GeometryFeatureNameEntry> names;
        names.pushBack(makeNameEntry(static_cast<uint32_t>(GE::TopologyType::Face), 3, "TopFace"));

        const Common::DynamicArray<uint8_t> payload = buildUpdateGeometryPayload(
            "Body", 42, topoData, sizeof(topoData) - 1, names);

        Platform::GeometryUpdateWire wire;
        ASSERT_TRUE(wire.parse(payload.data(), payload.size()));
        EXPECT_TRUE(wire.isValid());
        EXPECT_EQ(wire.getHeader().dataNodeId, 42u);
        EXPECT_STREQ(wire.getHeader().nodeName, "Body");
        ASSERT_EQ(wire.getTopoDataSize(), sizeof(topoData) - 1);
        EXPECT_EQ(std::memcmp(wire.getTopoData(), topoData, sizeof(topoData) - 1), 0);

        ASSERT_EQ(wire.getNameCount(), 1u);
        Platform::GeometryFeatureNameEntry entry{};
        ASSERT_TRUE(wire.getNameEntry(0, entry));
        EXPECT_EQ(entry.featureType, static_cast<uint32_t>(GE::TopologyType::Face));
        EXPECT_EQ(entry.featureIndex, 3u);
        EXPECT_STREQ(entry.name, "TopFace");
        EXPECT_FALSE(wire.getNameEntry(1, entry));
    }

    TEST(GeometryUpdateWire, rejectsMalformedPayloads)
    {
        const char topoData[] = "X";
        Common::DynamicArray<Platform::GeometryFeatureNameEntry> names;
        names.pushBack(makeNameEntry(static_cast<uint32_t>(GE::TopologyType::Edge), 0, "E0"));
        const Common::DynamicArray<uint8_t> payload =
            buildUpdateGeometryPayload("Body", 0, topoData, sizeof(topoData) - 1, names);

        Platform::GeometryUpdateWire wire;
        EXPECT_FALSE(wire.parse(nullptr, 0));
        EXPECT_FALSE(wire.isValid());
        EXPECT_FALSE(wire.parse(payload.data(), payload.size() - 1));

        Common::DynamicArray<uint8_t> corrupted = payload;
        corrupted[0] = 0; // breaks the magic
        EXPECT_FALSE(wire.parse(corrupted.data(), corrupted.size()));

        const Common::DynamicArray<uint8_t> emptyTopology =
            buildUpdateGeometryPayload("Body", 0, nullptr, 0, names);
        EXPECT_FALSE(wire.parse(emptyTopology.data(), emptyTopology.size()));

        Common::DynamicArray<Platform::GeometryFeatureNameEntry> unterminated;
        Platform::GeometryFeatureNameEntry entry{};
        entry.featureType = static_cast<uint32_t>(GE::TopologyType::Face);
        entry.featureIndex = 1;
        std::memset(entry.name, 'A', sizeof(entry.name));
        unterminated.pushBack(entry);
        const Common::DynamicArray<uint8_t> badName =
            buildUpdateGeometryPayload("Body", 0, topoData, sizeof(topoData) - 1, unterminated);
        EXPECT_FALSE(wire.parse(badName.data(), badName.size()));
    }

} // namespace
