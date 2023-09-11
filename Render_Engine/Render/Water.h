#pragma once

#include "libexport.h"

#include "Entity.h"


class DLLAPI Water : public Entity
{
public:
    Water(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale);
    Water(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale, int size);

    void Load_Texture(std::string filePath);

    void Update(float deltaTime);

private:
    int m_Size = 100;

    //std::vector<unsigned int> m_Edges;
    
    //std::vector<

    void Calc_Vertex(std::vector<Eigen::Vector3f> &vertices, std::vector<Eigen::Vector3f> &normals, std::vector<Eigen::Vector2f> &texcoords, std::vector<size_t> &indices);

};
