#include <gtest/gtest.h>

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Utils.hpp"
#include "DataBase/MeshTopology.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace
{
    using namespace EgLab;

    /// Five nodes, one Tri3 (1-2-3) and one Quad4 (2-3-4-5): node 1 belongs to the triangle
    /// only, node 5 to the quad only, nodes 2/3 to both.
    Common::SharedPtr<ME::Mesh> buildTopologyMesh()
    {
        auto mesh = Common::makeShared<ME::Mesh>();
        for (int index = 0; index < 5; ++index)
        {
            ME::Node node;
            node.setId(static_cast<ME::IdType>(index + 1));
            ME::CoordType coord(index, index * 2, index * 3);
            node.setXYZ(coord);
            mesh->addNode(Common::move(node));
        }

        ME::Elem triangle;
        triangle.setId(1);
        triangle.setType(ME::ElemType::Tri3);
        triangle.setNode(0, 1);
        triangle.setNode(1, 2);
        triangle.setNode(2, 3);
        mesh->addElem(Common::move(triangle));

        ME::Elem quad;
        quad.setId(2);
        quad.setType(ME::ElemType::Quad4);
        quad.setNode(0, 2);
        quad.setNode(1, 3);
        quad.setNode(2, 4);
        quad.setNode(3, 5);
        mesh->addElem(Common::move(quad));
        return mesh;
    }

    std::size_t findTypeCount(const Common::DynamicArray<Platform::MeshElementTypeCount>& counts,
                              ME::ElemType type)
    {
        for (std::size_t index = 0; index < counts.size(); ++index)
        {
            if (counts[index].type == static_cast<int32_t>(type))
            {
                return counts[index].count;
            }
        }
        return 0;
    }
} // namespace

TEST(MeshTopologyTest, CountsNodesAndElementTypes)
{
    Platform::MeshTopology topology(buildTopologyMesh());

    ASSERT_TRUE(topology.isValid());
    EXPECT_EQ(topology.getNodeCount(), 5u);
    EXPECT_EQ(topology.getElementCount(), 2u);

    const auto& counts = topology.getElementTypeCounts();
    ASSERT_EQ(counts.size(), 2u);
    EXPECT_EQ(findTypeCount(counts, ME::ElemType::Tri3), 1u);
    EXPECT_EQ(findTypeCount(counts, ME::ElemType::Quad4), 1u);
    EXPECT_EQ(findTypeCount(counts, ME::ElemType::Hex8), 0u);
}

TEST(MeshTopologyTest, CollectsNodeCoordinates)
{
    Platform::MeshTopology topology(buildTopologyMesh());

    Common::DynamicArray<Platform::MeshNodeRecord> nodes;
    topology.collectNodes(nodes);

    ASSERT_EQ(nodes.size(), 5u);
    EXPECT_EQ(nodes[0].id, 1u);
    EXPECT_FLOAT_EQ(nodes[0].x, 0.0f);
    EXPECT_FLOAT_EQ(nodes[0].y, 0.0f);
    EXPECT_FLOAT_EQ(nodes[0].z, 0.0f);
    EXPECT_EQ(nodes[4].id, 5u);
    EXPECT_FLOAT_EQ(nodes[4].x, 4.0f);
    EXPECT_FLOAT_EQ(nodes[4].y, 8.0f);
    EXPECT_FLOAT_EQ(nodes[4].z, 12.0f);
}

TEST(MeshTopologyTest, FiltersElementsByType)
{
    Platform::MeshTopology topology(buildTopologyMesh());

    Common::DynamicArray<Platform::MeshElementRecord> elements;

    topology.collectElements(-1, elements);
    EXPECT_EQ(elements.size(), 2u);

    topology.collectElements(static_cast<int32_t>(ME::ElemType::Tri3), elements);
    ASSERT_EQ(elements.size(), 1u);
    EXPECT_EQ(elements[0].id, 1u);
    EXPECT_EQ(elements[0].type, static_cast<int32_t>(ME::ElemType::Tri3));

    topology.collectElements(static_cast<int32_t>(ME::ElemType::Quad4), elements);
    ASSERT_EQ(elements.size(), 1u);
    EXPECT_EQ(elements[0].id, 2u);

    topology.collectElements(static_cast<int32_t>(ME::ElemType::Hex8), elements);
    EXPECT_TRUE(elements.empty());
}

TEST(MeshTopologyTest, ListsElementsReferencingNode)
{
    Platform::MeshTopology topology(buildTopologyMesh());

    Common::DynamicArray<Platform::MeshElementRecord> elements;

    topology.collectNodeElements(1, elements);
    ASSERT_EQ(elements.size(), 1u);
    EXPECT_EQ(elements[0].id, 1u);

    topology.collectNodeElements(2, elements);
    ASSERT_EQ(elements.size(), 2u);
    EXPECT_EQ(elements[0].id, 1u);
    EXPECT_EQ(elements[1].id, 2u);

    topology.collectNodeElements(5, elements);
    ASSERT_EQ(elements.size(), 1u);
    EXPECT_EQ(elements[0].id, 2u);

    topology.collectNodeElements(999, elements);
    EXPECT_TRUE(elements.empty());
}

TEST(MeshTopologyTest, ListsNodesOfElement)
{
    Platform::MeshTopology topology(buildTopologyMesh());

    Common::DynamicArray<Platform::MeshNodeRecord> nodes;

    topology.collectElementNodes(2, nodes);
    ASSERT_EQ(nodes.size(), 4u);
    EXPECT_EQ(nodes[0].id, 2u);
    EXPECT_EQ(nodes[3].id, 5u);
    EXPECT_FLOAT_EQ(nodes[3].x, 4.0f);
    EXPECT_FLOAT_EQ(nodes[3].y, 8.0f);
    EXPECT_FLOAT_EQ(nodes[3].z, 12.0f);

    topology.collectElementNodes(999, nodes);
    EXPECT_TRUE(nodes.empty());
}

TEST(MeshTopologyTest, HandlesMissingMesh)
{
    Common::SharedPtr<ME::Mesh> emptyMesh;
    Platform::MeshTopology topology(emptyMesh);

    EXPECT_FALSE(topology.isValid());
    EXPECT_EQ(topology.getNodeCount(), 0u);
    EXPECT_EQ(topology.getElementCount(), 0u);
    EXPECT_TRUE(topology.getElementTypeCounts().empty());

    Common::DynamicArray<Platform::MeshNodeRecord> nodes;
    topology.collectNodes(nodes);
    EXPECT_TRUE(nodes.empty());
}

TEST(MeshTopologyTest, NamesElementTypes)
{
    EXPECT_STREQ(Platform::MeshTopology::elementTypeName(static_cast<int32_t>(ME::ElemType::Tri3)),
                 "Tri3");
    EXPECT_STREQ(Platform::MeshTopology::elementTypeName(static_cast<int32_t>(ME::ElemType::Hex20)),
                 "Hex20");
    EXPECT_STREQ(Platform::MeshTopology::elementTypeName(12345), "Unknown");
}
