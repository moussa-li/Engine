#include "test_mesh.h"

#include "Common/SharedPtr.hpp"
#include "MeshEngine/Algorithm/ExtractSurface.hpp"
#include "MeshEngine/IO/GmshImporter.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
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
    do
    {
        auto &n = meshIt.currentNode();
        EXPECT_EQ(n.getId(), i);
        i++;
    } while (meshIt.nextNode());

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

TEST_F(TestMesh, importgmsh)
{
    EgLab::ME::GmshImporter importer(EgLab::getTestDataDir("daodan.msh"));

    auto mesh = importer.getMesh();
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