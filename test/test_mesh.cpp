#include "test_mesh.h"

#include "MeshEngine/MeshData/Mesh.hpp"

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