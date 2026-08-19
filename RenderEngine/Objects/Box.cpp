#include "Box.hpp"

#include "Core/RenderMesh.hpp"
#include "Core/VertexArray.hpp"

namespace EgLab::RE
{
    Box::Box()
    {
        createMesh();
    }

#if 0
    Common::Return Box::createMesh()
    {
        DynamicArray<CoordType> vertices;
        CoordType v1(-0.5, -0.5, 0);
        CoordType v2(0.5, -0.5, 0);
        CoordType v3(0, 0.5, 0);
        vertices.pushBack(v1);
        vertices.pushBack(v2);
        vertices.pushBack(v3);

        DynamicArray<TexCoordType> texcoords;
        DynamicArray<IdxType> indices;
        indices.pushBack(0);
        indices.pushBack(1);
        indices.pushBack(2);
        DynamicArray<CoordType> normals;
        DynamicArray<Texture*> tex;

        _meshes.pushBack(makeShared<RenderMesh>(vertices, normals, texcoords, indices, tex));

        return Common::Return::Succeed;
    }

#else
    Common::Return Box::createMesh()
    {
        // DynamicArray<CoordType> vertices;
        // DynamicArray<TexCoordType> texcoords;
        // DynamicArray<IdxType> indices;
        // DynamicArray<CoordType> normals;

        // float n[][3] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};

        // float p[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

        // float t[4][2] = {{1, 1}, {1, 0}, {0, 1}, {0, 0}};

        // float temp[3];
        // for (int i = 0; i < 6; i++)
        // {
        //     for (int j = 0; j < 4; ++j)
        //     {
        //         memcpy(temp, n[i], sizeof(float) * 3);
        //         size_t c = 0;
        //         for (int k = 0; k < 3 && c < 2; k++)
        //         {
        //             if (temp[k] == 0)
        //             {
        //                 temp[k] = p[j][c];
        //                 c++;
        //             }
        //         }
        //         Vector3f position(temp[0], temp[1], temp[2]);
        //         Vector3f normal(n[i][0], n[i][1], n[i][2]);
        //         Vector2f texCoord(t[j][0], t[j][1]);
        //         vertices.pushBack(position);
        //         normals.pushBack(normal);
        //         texcoords.pushBack(texCoord);
        //     }

        //     indices.pushBack(0 + i * 4);
        //     indices.pushBack(1 + i * 4);
        //     indices.pushBack(2 + i * 4);
        //     indices.pushBack(1 + i * 4);
        //     indices.pushBack(2 + i * 4);
        //     indices.pushBack(3 + i * 4);
        // }

        // DynamicArray<Texture*> tex;

        // for (auto& v : vertices)
        // {
        //     LOG(INFO) << v[0] << ", " << v[1] << ", " << v[2];
        // }

        // for (int i = 0; i < indices.size(); i += 3)
        // {
        //     LOG(INFO) << indices[i] << ", " << indices[i + 1] << ", " << indices[i + 2];
        // }

        // _meshes.pushBack(makeShared<RenderMesh>(vertices, normals, texcoords, indices, tex));

        return Common::Return::Succeed;
    }
#endif
    // void Box::draw() const
    // {
    // }

} // namespace EgLab::RE