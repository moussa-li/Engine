#include "Water.h"
#include "WaterBehaviour.h"

Water::Water(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale)
    : Entity(position,rotation,scale)
{
    std::vector<Eigen::Vector3f> vertices;
    std::vector<Eigen::Vector3f> normals;
    std::vector<Eigen::Vector2f> texcoords;
    std::vector<size_t> indices;
    Calc_Vertex(vertices, normals, texcoords, indices);
    Mesh* mesh = new Mesh(vertices, normals, texcoords, indices, {});
    m_Meshes.push_back(mesh);

    m_Behaviour = new PhyE::WaterBehaviour(&m_Transform, m_Size);
    m_Behaviour->Load_Mesh(m_Meshes);
    if (m_Window)
        m_Behaviour->Set_Window(m_Window);
}

Water::Water(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale, int size)
    : Entity(position,rotation,scale), m_Size(size)
{
    std::vector<Eigen::Vector3f> vertices;
    std::vector<Eigen::Vector3f> normals;
    std::vector<Eigen::Vector2f> texcoords;
    std::vector<size_t> indices;
    Calc_Vertex(vertices, normals, texcoords, indices);
    Mesh* mesh = new Mesh(vertices, normals, texcoords, indices, {});
    m_Meshes.push_back(mesh);

    m_Behaviour = new PhyE::WaterBehaviour(&m_Transform, m_Size);
    m_Behaviour->Load_Mesh(m_Meshes);
    if (m_Window)
        m_Behaviour->Set_Window(m_Window);
}

void Water::Load_Texture(std::string filePath)
{
    Texture* texture = new Texture(filePath, "texture_diffuse");
    //m_Meshed[0]->Clear_Texture();
    m_Meshes[0]->Insert_Texture(texture);

}

void Water::Update(float deltaTime)
{
    m_Behaviour->Update(0.015f);
}

void Water::Calc_Vertex(std::vector<Eigen::Vector3f>& vertices, std::vector<Eigen::Vector3f>& normals, std::vector<Eigen::Vector2f>& texcoords, std::vector<size_t>& indices)
{
    int n = m_Size;
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
}
