#include "Plane.h"

void Plane::Load_Texture(std::string filePath)
{
    Texture* texture = new Texture(filePath, "texture_diffuse");
    //m_Meshed[0]->Clear_Texture();
    m_Meshes[0]->Insert_Texture(texture);
}

void Plane::Calc_Vertex(std::vector<Eigen::Vector3f> &vertices, std::vector<Eigen::Vector3f> &normals, std::vector<Eigen::Vector2f> &texcoords, std::vector<size_t> &indices)
{
    m_Right.normalize();
    m_Normal.normalize();
    Eigen::Vector3f Front = m_Right.cross(m_Normal);
    Eigen::Vector3f vertex;
    Eigen::Vector3f normal;
    Eigen::Vector2f texcoord;
    vertex = (m_Width * m_Right) + (m_Height * Front);
    normal = m_Normal;
    texcoord = Eigen::Vector2f(1.f, 0.f);
    vertices.push_back(vertex);
    normals.push_back(normal);
    texcoords.push_back(texcoord);

    vertex = (m_Width * m_Right) - (m_Height * Front);
    normal = m_Normal;
    texcoord = Eigen::Vector2f(1.f, 1.f);
    vertices.push_back(vertex);
    normals.push_back(normal);
    texcoords.push_back(texcoord);

    vertex = -(m_Width * m_Right) + (m_Height * Front);
    normal = m_Normal;
    texcoord = Eigen::Vector2f(0.f, 0.f);
    vertices.push_back(vertex);
    normals.push_back(normal);
    texcoords.push_back(texcoord);

    vertex = - (m_Width * m_Right) - (m_Height * Front);
    normal = m_Normal;
    texcoord = Eigen::Vector2f(0.f, 1.f);
    vertices.push_back(vertex);
    normals.push_back(normal);
    texcoords.push_back(texcoord);

    indices = { 0,1,2,1,2,3 };
}

