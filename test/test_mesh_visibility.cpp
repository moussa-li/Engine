#include "Common/DynamicArray.hpp"
#include "Common/HashSet.hpp"
#include "Common/SharedPtr.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/MeshVisibility.hpp"
#include "gtest/gtest.h"

namespace
{
    void addNode(EgLab::ME::Mesh& mesh, EgLab::ME::IdType nodeId, double x, double y)
    {
        EgLab::ME::Node node;
        node.setId(nodeId);
        EgLab::ME::CoordType coordinate(x, y, 0.0);
        node.setXYZ(coordinate);
        mesh.addNode(EgLab::Common::move(node));
    }

    void addQuad(EgLab::ME::Mesh& mesh, EgLab::ME::IdType elemId, EgLab::ME::IdType a,
                 EgLab::ME::IdType b, EgLab::ME::IdType c, EgLab::ME::IdType d)
    {
        EgLab::ME::Elem elem;
        elem.setId(elemId);
        elem.setType(EgLab::ME::ElemType::Quad4);
        elem.setNode(0, a);
        elem.setNode(1, b);
        elem.setNode(2, c);
        elem.setNode(3, d);
        mesh.addElem(EgLab::Common::move(elem));
    }
} // namespace

TEST(MeshVisibility, hidesNodesOfHiddenElements)
{
    EgLab::ME::Mesh mesh;
    addNode(mesh, 1, 0.0, 0.0);
    addNode(mesh, 2, 1.0, 0.0);
    addNode(mesh, 3, 1.0, 1.0);
    addNode(mesh, 4, 0.0, 1.0);
    addQuad(mesh, 1, 1, 2, 3, 4);

    EgLab::Common::HashSet<EgLab::ME::IdType> hiddenElements;
    hiddenElements.insert(1);

    EgLab::Common::HashSet<EgLab::ME::IdType> hiddenNodes;
    EgLab::ME::MeshVisibility::collectNodeIdsHiddenByElements(mesh, hiddenElements, hiddenNodes);

    EXPECT_EQ(hiddenNodes.size(), 4u);
    for (EgLab::ME::IdType nodeId = 1; nodeId <= 4; ++nodeId)
    {
        EXPECT_NE(hiddenNodes.find(nodeId), hiddenNodes.end());
    }
}

TEST(MeshVisibility, keepsNodesSharedWithVisibleElements)
{
    EgLab::ME::Mesh mesh;
    addNode(mesh, 1, 0.0, 0.0);
    addNode(mesh, 2, 1.0, 0.0);
    addNode(mesh, 3, 1.0, 1.0);
    addNode(mesh, 4, 0.0, 1.0);
    addNode(mesh, 5, 2.0, 0.0);
    addNode(mesh, 6, 2.0, 1.0);
    addQuad(mesh, 1, 1, 2, 3, 4);
    addQuad(mesh, 2, 2, 5, 6, 3);

    EgLab::Common::HashSet<EgLab::ME::IdType> hiddenElements;
    hiddenElements.insert(1);

    EgLab::Common::HashSet<EgLab::ME::IdType> hiddenNodes;
    EgLab::ME::MeshVisibility::collectNodeIdsHiddenByElements(mesh, hiddenElements, hiddenNodes);

    // Nodes 2 and 3 are still used by element 2, so only 1 and 4 disappear.
    EXPECT_EQ(hiddenNodes.size(), 2u);
    EXPECT_NE(hiddenNodes.find(1), hiddenNodes.end());
    EXPECT_NE(hiddenNodes.find(4), hiddenNodes.end());
    EXPECT_EQ(hiddenNodes.find(2), hiddenNodes.end());
    EXPECT_EQ(hiddenNodes.find(3), hiddenNodes.end());
}

TEST(MeshVisibility, leavesIsolatedNodesVisible)
{
    EgLab::ME::Mesh mesh;
    addNode(mesh, 1, 0.0, 0.0);
    addNode(mesh, 2, 1.0, 0.0);
    addNode(mesh, 3, 1.0, 1.0);
    addNode(mesh, 4, 0.0, 1.0);
    addNode(mesh, 5, 5.0, 5.0);
    addQuad(mesh, 1, 1, 2, 3, 4);

    EgLab::Common::HashSet<EgLab::ME::IdType> hiddenElements;
    hiddenElements.insert(1);

    EgLab::Common::HashSet<EgLab::ME::IdType> hiddenNodes;
    EgLab::ME::MeshVisibility::collectNodeIdsHiddenByElements(mesh, hiddenElements, hiddenNodes);

    EXPECT_EQ(hiddenNodes.find(5), hiddenNodes.end());
}

TEST(MeshVisibility, reportsNothingWhenNoElementIsHidden)
{
    EgLab::ME::Mesh mesh;
    addNode(mesh, 1, 0.0, 0.0);
    addNode(mesh, 2, 1.0, 0.0);
    addNode(mesh, 3, 1.0, 1.0);
    addNode(mesh, 4, 0.0, 1.0);
    addQuad(mesh, 1, 1, 2, 3, 4);

    EgLab::Common::HashSet<EgLab::ME::IdType> hiddenElements;
    EgLab::Common::HashSet<EgLab::ME::IdType> hiddenNodes;
    EgLab::ME::MeshVisibility::collectNodeIdsHiddenByElements(mesh, hiddenElements, hiddenNodes);

    EXPECT_TRUE(hiddenNodes.empty());
}
