#include "test_mesh.h"

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "MeshEngine/Algorithm/ExtractSurface.hpp"
#include "MeshEngine/IO/GmshImporter.hpp"
#include "MeshEngine/IO/GrdImporter.hpp"
#include "MeshEngine/MeshCheck/TypeChecker.hpp"
#include "MeshEngine/MeshData/HEMesh.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/MeshToHEMesh.hpp"
#include "MeshEngine/MeshData/MeshToPOD.hpp"
#include "test_utils.hpp"

TEST_F(TestMesh, createMesh)
{
    EgLab::ME::Mesh mesh;

    for (int i = 0; i < 3; i++)
    {
        EgLab::ME::Node n;
        n.setId(i + 1);
        EgLab::ME::CoordType c(i % 2, i % 3, 0);
        n.setXYZ(c);
        mesh.addNode(EgLab::Common::move(n));
    }

    EgLab::ME::Elem e;
    e.setId(1);
    e.setType(EgLab::ME::ElemType::Tri3);
    e.setNode(0, 1);
    e.setNode(1, 2);
    e.setNode(2, 3);
    mesh.addElem(EgLab::Common::move(e));

    EgLab::ME::MeshIterator meshIt(mesh);

    int i = 1;
    for (; meshIt.hasMoreNode(); meshIt.nextNode())
    {
        auto &n = meshIt.currentNode();
        EXPECT_EQ(n.getId(), i);
        i++;
    }

    auto &elem = meshIt.currentElem();
    EXPECT_EQ(elem.getId(), 1);
}

TEST_F(TestMesh, extraceSurface)
{
    EgLab::Common::SharedPtr<EgLab::ME::Mesh> mesh = EgLab::Common::makeShared<EgLab::ME::Mesh>();

    for (int i = 0; i < 5; i++)
    {
        EgLab::ME::Node n;
        n.setId(i + 1);
        EgLab::ME::CoordType c(i % 2, i % 3, 0);
        n.setXYZ(c);
        mesh->addNode(EgLab::Common::move(n));
    }

    EgLab::ME::Elem e;
    e.setId(1);
    e.setType(EgLab::ME::ElemType::Tet4);
    e.setNode(0, 1);
    e.setNode(1, 2);
    e.setNode(2, 3);
    e.setNode(3, 4);
    mesh->addElem(EgLab::Common::move(e));

    EgLab::ME::Elem e2;
    e2.setId(1);
    e2.setType(EgLab::ME::ElemType::Tet4);
    e2.setNode(0, 3);
    e2.setNode(1, 2);
    e2.setNode(2, 1);
    e2.setNode(3, 5);
    mesh->addElem(EgLab::Common::move(e2));

    EgLab::ME::ExtractSurface extractor(mesh);
    auto idxs = extractor.getSurface();
}

TEST_F(TestMesh, elemFaceGetterSupportsAllElementTypes)
{
    using EgLab::ME::Elem;
    using EgLab::ME::ElemFaceGetter;
    using EgLab::ME::ElemType;

    struct ElemFaceTestCase
    {
        ElemType type;
        int faceNumber;
        int faceNodeNumbers[6];
        int faceMaps[6][8];
    };
    EgLab::Common::DynamicArray<ElemFaceTestCase> cases;
    cases.pushBack({ElemType::Tri3, 1, {3}, {{0, 1, 2}}});
    cases.pushBack({ElemType::Tri6, 1, {6}, {{0, 1, 2, 3, 4, 5}}});
    cases.pushBack({ElemType::Quad4, 1, {4}, {{0, 1, 2, 3}}});
    cases.pushBack({ElemType::Quad8, 1, {8}, {{0, 1, 2, 3, 4, 5, 6, 7}}});
    cases.pushBack({ElemType::Tet4, 4, {3, 3, 3, 3}, {{0, 1, 2}, {0, 2, 3}, {0, 3, 1}, {1, 3, 2}}});
    cases.pushBack(
        {ElemType::Tet10,
         4,
         {6, 6, 6, 6},
         {{0, 1, 2, 4, 5, 6}, {0, 2, 3, 6, 9, 7}, {0, 3, 1, 7, 8, 4}, {1, 3, 2, 8, 9, 5}}});
    cases.pushBack({ElemType::Wed6,
                    5,
                    {3, 3, 4, 4, 4},
                    {{0, 2, 1}, {3, 4, 5}, {0, 1, 4, 3}, {1, 2, 5, 4}, {2, 0, 3, 5}}});
    cases.pushBack({ElemType::Prd5,
                    5,
                    {4, 3, 3, 3, 3},
                    {{0, 1, 2, 3}, {0, 1, 4}, {1, 2, 4}, {2, 3, 4}, {3, 0, 4}}});
    cases.pushBack(
        {ElemType::Hex8,
         6,
         {4, 4, 4, 4, 4, 4},
         {{0, 1, 2, 3}, {4, 7, 6, 5}, {0, 1, 5, 4}, {1, 2, 6, 5}, {2, 3, 7, 6}, {3, 0, 4, 7}}});
    cases.pushBack({ElemType::Hex20,
                    6,
                    {8, 8, 8, 8, 8, 8},
                    {{0, 1, 2, 3, 8, 9, 10, 11},
                     {4, 7, 6, 5, 15, 14, 13, 12},
                     {0, 1, 5, 4, 8, 17, 12, 16},
                     {1, 2, 6, 5, 9, 18, 13, 17},
                     {2, 3, 7, 6, 10, 19, 14, 18},
                     {3, 0, 4, 7, 11, 16, 15, 19}}});

    for (size_t caseIndex = 0; caseIndex < cases.size(); ++caseIndex)
    {
        const auto &testCase = cases[caseIndex];
        Elem element;
        element.setType(testCase.type);
        for (EgLab::ME::IdxType node = 0; node < element.getNodes().size(); ++node)
        {
            element.setNode(node, 100 + node);
        }

        ElemFaceGetter getter(element);
        ASSERT_EQ(getter.elemFaceNumber(), testCase.faceNumber);
        for (EgLab::ME::IdxType face = 0; face < testCase.faceNumber; ++face)
        {
            const int faceNodeNumber = testCase.faceNodeNumbers[face];
            EXPECT_EQ(getter.elemFaceNodeNumber(face), faceNodeNumber);

            for (int node = 0; node < faceNodeNumber; ++node)
            {
                const auto nodeId = getter.elemFace(face, node);
                EXPECT_EQ(nodeId, 100u + testCase.faceMaps[face][node]);
            }

            const auto triangles = getter.getNodes(face);
            EXPECT_EQ(triangles.size(), faceNodeNumber == 3   ? 3u
                                        : faceNodeNumber == 4 ? 6u
                                        : faceNodeNumber == 6 ? 12u
                                                              : 18u);
            const auto lines = getter.getLineNodes(face);
            EXPECT_EQ(lines.size(), faceNodeNumber == 3   ? 6u
                                    : faceNodeNumber == 4 ? 8u
                                    : faceNodeNumber == 6 ? 12u
                                                          : 16u);
        }
    }
}

TEST_F(TestMesh, importgmsh)
{
    EgLab::ME::GmshImporter importer(EgLab::getTestDataDir("daodan.msh"));

    auto mesh = importer.getMesh();
}

TEST_F(TestMesh, importGrd)
{
    EgLab::ME::GrdImporter importer(EgLab::getTestDataDir("simple.grd"));
    auto mesh = importer.getMesh();

    ASSERT_EQ(mesh->getNodeNumber(), 4);
    ASSERT_EQ(mesh->getElemNumber(), 1);
    EXPECT_DOUBLE_EQ(mesh->getNodeById(2).x(), 1.0);
    EXPECT_EQ(mesh->getElemById(10).getType(), EgLab::ME::ElemType::Tet4);
    EXPECT_EQ(mesh->getElemById(10).getNode(3), 4);
}

TEST_F(TestMesh, meshPODRoundTrip)
{
    auto mesh = EgLab::Common::makeShared<EgLab::ME::Mesh>();

    EgLab::ME::Node n1;
    n1.setId(1);
    EgLab::ME::CoordType c1(0.0, 0.0, 0.0);
    n1.setXYZ(c1);
    mesh->addNode(EgLab::Common::move(n1));

    EgLab::ME::Node n2;
    n2.setId(2);
    EgLab::ME::CoordType c2(1.0, 0.0, 0.0);
    n2.setXYZ(c2);
    mesh->addNode(EgLab::Common::move(n2));

    EgLab::ME::Node n3;
    n3.setId(3);
    EgLab::ME::CoordType c3(0.0, 1.0, 0.0);
    n3.setXYZ(c3);
    mesh->addNode(EgLab::Common::move(n3));

    EgLab::ME::Elem e;
    e.setId(10);
    e.setType(EgLab::ME::ElemType::Tri3);
    e.setNode(0, 1);
    e.setNode(1, 2);
    e.setNode(2, 3);
    mesh->addElem(EgLab::Common::move(e));

    auto pod = EgLab::ME::MeshToPOD::convert(mesh);
    auto restored = EgLab::ME::MeshToPOD::deserialize(pod);

    EXPECT_EQ(restored->getNodeNumber(), 3);
    EXPECT_EQ(restored->getElemNumber(), 1);

    auto &rNode1 = restored->getNodeById(1);
    EXPECT_DOUBLE_EQ(rNode1.x(), 0.0);
    EXPECT_DOUBLE_EQ(rNode1.y(), 0.0);
    EXPECT_DOUBLE_EQ(rNode1.z(), 0.0);

    auto &rElem = restored->getElemById(10);
    EXPECT_EQ(rElem.getType(), EgLab::ME::ElemType::Tri3);
    EXPECT_EQ(rElem.getNode(0), 1);
    EXPECT_EQ(rElem.getNode(1), 2);
    EXPECT_EQ(rElem.getNode(2), 3);
}

TEST_F(TestMesh, importgmsh2)
{
    EgLab::ME::GmshImporter importer(EgLab::getTestDataDir("B27234.msh"));

    auto mesh = importer.getMesh();
}

TEST_F(TestMesh, halfEdgeFaceCreatesCyclicIndices)
{
    EgLab::ME::HEMesh mesh;
    auto v0 = mesh.createVertex(0.0f, 0.0f, 0.0f);
    auto v1 = mesh.createVertex(1.0f, 0.0f, 0.0f);
    auto v2 = mesh.createVertex(0.0f, 1.0f, 0.0f);
    EgLab::Common::DynamicArray<EgLab::ME::IdType> faceVertices;
    faceVertices.pushBack(v0);
    faceVertices.pushBack(v1);
    faceVertices.pushBack(v2);

    EXPECT_EQ(mesh.createFace(faceVertices), EgLab::Common::Return::Succeed);
    EXPECT_EQ(mesh.getVertexCount(), 3u);
    EXPECT_EQ(mesh.getFaceCount(), 1u);
    EXPECT_EQ(mesh.getHalfEdgeCount(), 3u);
    EXPECT_EQ(mesh.getFace(0)._halfEdge, 0u);
    EXPECT_EQ(mesh.getHalfEdge(0).next, 1u);
    EXPECT_EQ(mesh.getHalfEdge(0).prev, 2u);
    EXPECT_EQ(mesh.getHalfEdge(0).twin, EgLab::ME::INVALID_IDX);
    EXPECT_EQ(mesh.getVertex(0)._halfEdge, 0u);
}

TEST_F(TestMesh, halfEdgeFacesLinkTwinsByIndex)
{
    EgLab::ME::HEMesh mesh;
    auto v0 = mesh.createVertex(0.0f, 0.0f, 0.0f);
    auto v1 = mesh.createVertex(1.0f, 0.0f, 0.0f);
    auto v2 = mesh.createVertex(1.0f, 1.0f, 0.0f);
    auto v3 = mesh.createVertex(0.0f, 1.0f, 0.0f);
    EgLab::Common::DynamicArray<EgLab::ME::IdType> firstFace;
    firstFace.pushBack(v0);
    firstFace.pushBack(v1);
    firstFace.pushBack(v2);
    EgLab::Common::DynamicArray<EgLab::ME::IdType> secondFace;
    secondFace.pushBack(v0);
    secondFace.pushBack(v2);
    secondFace.pushBack(v3);

    ASSERT_EQ(mesh.createFace(firstFace), EgLab::Common::Return::Succeed);
    ASSERT_EQ(mesh.createFace(secondFace), EgLab::Common::Return::Succeed);
    EXPECT_EQ(mesh.getHalfEdge(2).twin, 3u);
    EXPECT_EQ(mesh.getHalfEdge(3).twin, 2u);
    EXPECT_EQ(mesh.getHalfEdge(3).face, 1u);
}

TEST_F(TestMesh, halfEdgeRejectsInvalidFaceWithoutMutation)
{
    EgLab::ME::HEMesh mesh;
    auto v0 = mesh.createVertex(0.0f, 0.0f, 0.0f);
    auto v1 = mesh.createVertex(1.0f, 0.0f, 0.0f);
    EgLab::Common::DynamicArray<EgLab::ME::IdType> invalidFace;
    invalidFace.pushBack(v0);
    invalidFace.pushBack(v1);
    invalidFace.pushBack(999u);

    EXPECT_EQ(mesh.createFace(invalidFace), EgLab::Common::Return::BadInput);
    EXPECT_EQ(mesh.getFaceCount(), 0u);
    EXPECT_EQ(mesh.getHalfEdgeCount(), 0u);
}

TEST_F(TestMesh, convertMeshToHalfEdgeMeshPreservesNodeIds)
{
    auto mesh = EgLab::Common::makeShared<EgLab::ME::Mesh>();
    const EgLab::ME::IdType nodeIds[] = {10, 20, 30};
    const EgLab::ME::CoordType coordinates[] = {EgLab::ME::CoordType(0.0, 0.0, 0.0),
                                                EgLab::ME::CoordType(1.0, 0.0, 0.0),
                                                EgLab::ME::CoordType(0.0, 1.0, 0.0)};
    for (size_t index = 0; index < 3; ++index)
    {
        EgLab::ME::Node node;
        node.setId(nodeIds[index]);
        auto coordinate = coordinates[index];
        node.setXYZ(coordinate);
        ASSERT_EQ(mesh->addNode(EgLab::Common::move(node)), EgLab::Common::Return::Succeed);
    }

    EgLab::ME::Elem triangle;
    triangle.setId(1);
    triangle.setType(EgLab::ME::ElemType::Tri3);
    triangle.setNode(0, nodeIds[0]);
    triangle.setNode(1, nodeIds[1]);
    triangle.setNode(2, nodeIds[2]);
    ASSERT_EQ(mesh->addElem(EgLab::Common::move(triangle)), EgLab::Common::Return::Succeed);

    EgLab::Common::SharedPtr<EgLab::ME::HEMesh> halfEdgeMesh;
    ASSERT_EQ(EgLab::ME::MeshToHEMesh::convert(mesh, halfEdgeMesh), EgLab::Common::Return::Succeed);
    ASSERT_NE(halfEdgeMesh, nullptr);
    EXPECT_EQ(halfEdgeMesh->getVertexCount(), 3u);
    EXPECT_EQ(halfEdgeMesh->getFaceCount(), 1u);
    EXPECT_EQ(halfEdgeMesh->getHalfEdgeCount(), 3u);
    EXPECT_EQ(halfEdgeMesh->getVertex(1)._id, 20u);
    EXPECT_EQ(halfEdgeMesh->getVertexIndex(30), 2u);
}

TEST_F(TestMesh, convertTetMeshToHalfEdgeBoundary)
{
    auto mesh = EgLab::Common::makeShared<EgLab::ME::Mesh>();
    for (EgLab::ME::IdType id = 1; id <= 4; ++id)
    {
        EgLab::ME::Node node;
        node.setId(id);
        EgLab::ME::CoordType coordinate(static_cast<double>(id == 2), static_cast<double>(id == 3),
                                        static_cast<double>(id == 4));
        node.setXYZ(coordinate);
        ASSERT_EQ(mesh->addNode(EgLab::Common::move(node)), EgLab::Common::Return::Succeed);
    }

    EgLab::ME::Elem tetrahedron;
    tetrahedron.setId(1);
    tetrahedron.setType(EgLab::ME::ElemType::Tet4);
    for (EgLab::ME::IdxType index = 0; index < 4; ++index)
    {
        tetrahedron.setNode(index, index + 1);
    }
    ASSERT_EQ(mesh->addElem(EgLab::Common::move(tetrahedron)), EgLab::Common::Return::Succeed);

    EgLab::Common::SharedPtr<EgLab::ME::HEMesh> halfEdgeMesh;
    ASSERT_EQ(EgLab::ME::MeshToHEMesh::convert(mesh, halfEdgeMesh), EgLab::Common::Return::Succeed);
    ASSERT_NE(halfEdgeMesh, nullptr);
    EXPECT_EQ(halfEdgeMesh->getVertexCount(), 4u);
    EXPECT_EQ(halfEdgeMesh->getFaceCount(), 4u);
    EXPECT_EQ(halfEdgeMesh->getHalfEdgeCount(), 12u);
}

TEST_F(TestMesh, typeChecker)
{
    EgLab::ME::GmshImporter importer(EgLab::getTestDataDir("daodan.msh"));

    auto mesh = importer.getMesh(2);
    EgLab::ME::TypeChecker checker(mesh);
    EXPECT_EQ(checker.allTypeIs(EgLab::ME::ElemType::Quad4), false);
}

TEST_F(TestMesh, midSurfGen)
{
    EgLab::ME::GmshImporter importer(EgLab::getTestDataDir("daodan.msh"));

    auto mesh = importer.getMesh();
}