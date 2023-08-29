#include "Plane.h"

void Plane::Load_Texture(std::string filePath)
{
    Texture* texture = new Texture(filePath, "texture_diffuse");
    //m_Meshed[0]->Clear_Texture();
    m_Meshes[0]->Insert_Texture(texture);
}

void Plane::Calc_Vertex(std::vector<Vertex> &vertex, std::vector<size_t> &indices)
{
    m_Right.normalize();
    m_Normal.normalize();
    Eigen::Vector3f Front = m_Right.cross(m_Normal);
    Vertex v;
    v.Position = m_Transform.GetPosition() + (m_Width * m_Right) + (m_Height * Front);
    v.Normal = m_Normal;
    v.TexCoords = Eigen::Vector2f(1.f, 0.f);
    vertex.push_back(v);

    v.Position = m_Transform.GetPosition() + (m_Width * m_Right) - (m_Height * Front);
    v.Normal = m_Normal;
    v.TexCoords = Eigen::Vector2f(1.f, 1.f);
    vertex.push_back(v);

    v.Position = m_Transform.GetPosition() - (m_Width * m_Right) + (m_Height * Front);
    v.Normal = m_Normal;
    v.TexCoords = Eigen::Vector2f(0.f, 0.f);
    vertex.push_back(v);

    v.Position = m_Transform.GetPosition() - (m_Width * m_Right) - (m_Height * Front);
    v.Normal = m_Normal;
    v.TexCoords = Eigen::Vector2f(0.f, 1.f);
    vertex.push_back(v);

    indices = { 0,1,2,1,2,3 };
}

