#include "Cloth.h"

Cloth::Cloth(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale)
    : Entity(position,rotation,scale)
{
    std::vector<Eigen::Vector3f> vertices;
    std::vector<Eigen::Vector3f> normals;
    std::vector<Eigen::Vector2f> texcoords;
    std::vector<size_t> indices;
    Calc_Vertex(vertices, normals, texcoords, indices);
    Mesh* mesh = new Mesh(vertices, normals, texcoords, indices, {});
    m_Meshes.push_back(mesh);


    m_Behaviour = new PhyE::ClothBehaviour(&m_Transform);
    m_Behaviour->Load_Mesh(m_Meshes);
    if (m_Window)
        m_Behaviour->Set_Window(m_Window);
}

void Cloth::Load_Texture(std::string filePath)
{
    Texture* texture = new Texture(filePath, "texture_diffuse");
    //m_Meshed[0]->Clear_Texture();
    m_Meshes[0]->Insert_Texture(texture);
}

void Cloth::Update(float deltaTime)
{
    m_Behaviour->Update(0.015f);
}

void Cloth::Calc_Vertex(std::vector<Eigen::Vector3f>& vertices, std::vector<Eigen::Vector3f>& normals, std::vector<Eigen::Vector2f>& texcoords, std::vector<size_t>& indices)
{
    int n = 21;
    vertices.resize(n * n);
    texcoords.resize(n * n);
    normals.resize(n * n);
    indices.resize((n - 1) * (n - 1) * 6);

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            vertices[i * n + j] = Eigen::Vector3f(5 - 10.f * j / (n - 1), 0, 5 - 10.f * i / (n - 1));
            texcoords[i * n + j] = Eigen::Vector2f(j / (n - 1.f), i / (n - 1.f));
            normals[i * n + j] = Eigen::Vector3f(0, 1, 0);
        }
    }


    for (int i = 0, t = 0; i < n - 1; ++i)
    {
        for (int j = 0; j < n - 1; ++j, ++t)
        {
            indices[t * 6 + 0] = i * n + j;
            indices[t * 6 + 1] = i * n + (j+1);
            indices[t * 6 + 2] = (i+1) * n + (j+1);
            indices[t * 6 + 3] = i * n + j;
            indices[t * 6 + 4] = (i+1) * n + (j+1);
            indices[t * 6 + 5] = (i+1) * n + j;
        }
    }

    /*std::vector<unsigned int> edges(indices.size()*2);
    for (int i = 0; i < indices.size(); ++i)
    {
        edges[i * 2 + 0] = indices[i + 0];
        edges[i * 2 + 1] = indices[i + 1];
        edges[i * 2 + 2] = indices[i + 2];
        edges[i * 2 + 3] = indices[i + 3];
        edges[i * 2 + 4] = indices[i + 4];
        edges[i * 2 + 5] = indices[i + 5];
    }

    for (int i = 0; i < edges.size(); i += 2)
    {
        if(edges)
    }*/

}
