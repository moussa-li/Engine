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
        std::vector<Vertex> vertex;
        std::vector<size_t> indices;
        Calc_Vertex(vertex, indices);
        //Texture* texture = new Texture(path, type);
        Mesh* mesh = new Mesh(vertex, indices, {});
        m_Meshes.push_back(mesh);
    }

    void Load_Texture(std::string filePath);

    void Calc_Vertex(std::vector<Vertex> &vertex, std::vector<size_t> &indices);

private:
    void Load(const std::string & fileDir){}

};
