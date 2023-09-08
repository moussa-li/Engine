#pragma once

#include <eigen3/Eigen/Eigen>

#include "libexport.h"

#include "Entity.h"

class DLLAPI Plane : public Entity
{
private:
    Eigen::Vector3f m_Normal;
    Eigen::Vector3f m_Right;
    float m_Width;
    float m_Height;

public:
    Plane(Eigen::Vector3f position, Eigen::Vector3f normal, Eigen::Vector3f right, float width, float height)
        : Entity(position, Eigen::Vector3f(0,0,0), Eigen::Vector3f(1.f,1.f,1.f)), m_Normal(normal), m_Right(right), m_Width(width), m_Height(height)
    {
        std::vector<Eigen::Vector3f> vertices;
        std::vector<Eigen::Vector3f> normals;
        std::vector<Eigen::Vector2f> texcoords;
        std::vector<size_t> indices;
        Calc_Vertex(vertices, normals, texcoords, indices);
        //Texture* texture = new Texture(path, type);
        Mesh* mesh = new Mesh(vertices, normals, texcoords, indices, {});
        m_Meshes.push_back(mesh);
    }

    void Load_Texture(std::string filePath);

    void Calc_Vertex(std::vector<Eigen::Vector3f> &vertices, std::vector<Eigen::Vector3f> &normals, std::vector<Eigen::Vector2f> &texcoords, std::vector<size_t> &indices);

private:
    // Load what ? why you are here?
    void Load(const std::string & fileDir){}

};
