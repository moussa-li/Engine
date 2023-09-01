#include "Cube.h"


void Cube::Calc_Vertex(std::vector<Eigen::Vector3f> &vertices, std::vector<Eigen::Vector3f> &normals, std::vector<Eigen::Vector2f> &texcoords, std::vector<size_t> &indices)
{
    vertices = {};
    normals = {};
    texcoords = {};
    indices = {};
    //Vertex v;

    float n[][3] = { {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1}, {0,0,-1} };
    std::vector<std::vector<float>> p = { {-1,-1},{-1,1},{1,-1},{1,1} };
    std::vector<Eigen::Vector2f> t = { Eigen::Vector2f(1,1),Eigen::Vector2f(1,0),Eigen::Vector2f(0,1),Eigen::Vector2f(0,0) };

    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            std::vector<float> tmpP = p[j];
            for (int k = 0; k < 3; ++k)
            {
                if (n[i][k] != 0)
                {
                    tmpP.insert(tmpP.begin() + k, n[i][k]);
                    break;
                }
            }
            Eigen::Vector3f Position =  Size / 2 * Eigen::Vector3f(tmpP[0], tmpP[1], tmpP[2]);
            Eigen::Vector3f Normal = Eigen::Vector3f(n[i][0],n[i][1],n[i][2]);
            Eigen::Vector2f TexCoord = t[j];
            vertices.push_back(Position);
            normals.push_back(Normal);
            texcoords.push_back(TexCoord);
            //vertex.push_back(v);
        }
        indices.push_back(0 + i * 4);
        indices.push_back(1 + i * 4);
        indices.push_back(2 + i * 4);
        indices.push_back(1 + i * 4);
        indices.push_back(2 + i * 4);
        indices.push_back(3 + i * 4);
    }



}
