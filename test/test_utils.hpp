#pragma once
#include <filesystem>

#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab
{

    inline Common::String getTestDataDir(const Common::String &file)
    {
        std::filesystem::path root(__FILE__);
        root = root.parent_path();
        root /= "testdata";
        root /= file.c_str();
        return Common::String(root.generic_string().c_str());
    }

    /// Builds two quadratic (Tet10) tetrahedra that share the face (1, 2, 3). Element 2 is
    /// the mirror of element 1 across that face, so the shared face is the only internal
    /// face and the full boundary consists of six quadratic triangle faces.
    inline Common::SharedPtr<ME::Mesh> makeTwoTet10Mesh()
    {
        auto mesh = Common::makeShared<ME::Mesh>();
        const double coordinates[14][3] = {
            // Corners of element 1.
            {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
            // Apex of element 2 mirrored across the (1, 2, 3) face.
            {0, 0, -1},
            // Midside nodes of element 1: edges (1,2) (2,3) (3,1) (1,4) (2,4) (3,4).
            {0.5, 0, 0}, {0.5, 0.5, 0}, {0, 0.5, 0}, {0, 0, 0.5}, {0.5, 0, 0.5},
            {0, 0.5, 0.5},
            // Midside nodes of element 2: edges (1,5) (2,5) (3,5).
            {0, 0, -0.5}, {0.5, 0, -0.5}, {0, 0.5, -0.5},
        };
        for (int index = 0; index < 14; ++index)
        {
            ME::Node node;
            node.setId(index + 1);
            ME::CoordType coordinate(coordinates[index][0], coordinates[index][1],
                                     coordinates[index][2]);
            node.setXYZ(coordinate);
            mesh->addNode(Common::move(node));
        }

        const ME::IdType tet1Nodes[10] = {1, 2, 3, 4, 6, 7, 8, 9, 10, 11};
        const ME::IdType tet2Nodes[10] = {1, 2, 3, 5, 6, 7, 8, 12, 13, 14};
        const ME::IdType *elementNodes[2] = {tet1Nodes, tet2Nodes};

        for (int element = 0; element < 2; ++element)
        {
            ME::Elem elem;
            elem.setId(element + 1);
            elem.setType(ME::ElemType::Tet10);
            for (int node = 0; node < 10; ++node)
            {
                elem.setNode(node, elementNodes[element][node]);
            }
            mesh->addElem(Common::move(elem));
        }
        return mesh;
    }
} // namespace EgLab
