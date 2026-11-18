#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "Work/SelectionModel.hpp"
#include "gtest/gtest.h"

namespace
{
    EgLab::Common::SharedPtr<EgLab::ME::Mesh> buildQuadWithIsolatedNode()
    {
        auto mesh = EgLab::Common::makeShared<EgLab::ME::Mesh>();

        for (int index = 0; index < 5; ++index)
        {
            EgLab::ME::Node node;
            node.setId(static_cast<EgLab::ME::IdType>(index + 1));
            EgLab::ME::CoordType coordinate(static_cast<double>(index % 2),
                                            static_cast<double>(index / 2), 0.0);
            node.setXYZ(coordinate);
            mesh->addNode(EgLab::Common::move(node));
        }

        EgLab::ME::Elem elem;
        elem.setId(1);
        elem.setType(EgLab::ME::ElemType::Quad4);
        elem.setNode(0, 1);
        elem.setNode(1, 2);
        elem.setNode(2, 3);
        elem.setNode(3, 4);
        mesh->addElem(EgLab::Common::move(elem));
        return mesh;
    }

    bool containsId(const EgLab::Common::DynamicArray<uint32_t>& ids, uint32_t id)
    {
        for (size_t index = 0; index < ids.size(); ++index)
        {
            if (ids[index] == id)
            {
                return true;
            }
        }
        return false;
    }
} // namespace

TEST(Selection, classifiesAssociatedAndIsolatedNodes)
{
    auto mesh = buildQuadWithIsolatedNode();

    EgLab::Platform::SelectionModel selection;
    EgLab::Common::DynamicArray<uint32_t> nodeIds;
    nodeIds.pushBack(1); // referenced by element 1
    nodeIds.pushBack(5); // referenced by no element
    selection.addMeshSelection(mesh, nodeIds, EgLab::Common::DynamicArray<uint32_t>());

    EgLab::Platform::SelectionSnapshot snapshot;
    selection.getSnapshot(snapshot);

    ASSERT_EQ(snapshot.meshes.size(), 1u);
    const auto& meshSnapshot = snapshot.meshes[0];
    EXPECT_EQ(meshSnapshot.nodeIds.size(), 2u);
    EXPECT_TRUE(containsId(meshSnapshot.associatedElementIds, 1));
    EXPECT_EQ(meshSnapshot.isolatedNodeIds.size(), 1u);
    EXPECT_TRUE(containsId(meshSnapshot.isolatedNodeIds, 5));
    EXPECT_EQ(snapshot.totalNodes, 2u);
    EXPECT_EQ(snapshot.totalElements, 0u);
}

TEST(Selection, keepsElementSelectionSeparateFromNodes)
{
    auto mesh = buildQuadWithIsolatedNode();

    EgLab::Platform::SelectionModel selection;
    EgLab::Common::DynamicArray<uint32_t> elementIds;
    elementIds.pushBack(1);
    selection.addMeshSelection(mesh, EgLab::Common::DynamicArray<uint32_t>(), elementIds);

    EgLab::Platform::SelectionSnapshot snapshot;
    selection.getSnapshot(snapshot);

    ASSERT_EQ(snapshot.meshes.size(), 1u);
    EXPECT_TRUE(snapshot.meshes[0].nodeIds.empty());
    EXPECT_TRUE(snapshot.meshes[0].isolatedNodeIds.empty());
    EXPECT_TRUE(containsId(snapshot.meshes[0].elementIds, 1));
    EXPECT_EQ(snapshot.totalElements, 1u);
}

TEST(Selection, prunesEntitiesRemovedFromTheMesh)
{
    auto mesh = buildQuadWithIsolatedNode();

    EgLab::Platform::SelectionModel selection;
    EgLab::Common::DynamicArray<uint32_t> nodeIds;
    nodeIds.pushBack(1);
    nodeIds.pushBack(5);
    selection.addMeshSelection(mesh, nodeIds, EgLab::Common::DynamicArray<uint32_t>());

    mesh->delNode(5);
    selection.pruneMesh(mesh);

    EgLab::Platform::SelectionSnapshot snapshot;
    selection.getSnapshot(snapshot);

    ASSERT_EQ(snapshot.meshes.size(), 1u);
    EXPECT_EQ(snapshot.meshes[0].nodeIds.size(), 1u);
    EXPECT_TRUE(containsId(snapshot.meshes[0].nodeIds, 1));
    EXPECT_TRUE(snapshot.meshes[0].isolatedNodeIds.empty());
}

TEST(Selection, clearDropsEveryMeshSelection)
{
    auto mesh = buildQuadWithIsolatedNode();

    EgLab::Platform::SelectionModel selection;
    EgLab::Common::DynamicArray<uint32_t> nodeIds;
    nodeIds.pushBack(5);
    selection.addMeshSelection(mesh, nodeIds, EgLab::Common::DynamicArray<uint32_t>());
    ASSERT_FALSE(selection.empty());

    selection.clear();

    EgLab::Platform::SelectionSnapshot snapshot;
    selection.getSnapshot(snapshot);
    EXPECT_TRUE(selection.empty());
    EXPECT_TRUE(snapshot.meshes.empty());
    EXPECT_EQ(snapshot.totalNodes, 0u);
}
