#pragma once

#include "libexport.h"

#include "Entity.h"
#include "ClothBehaviour.h"

class DLLAPI Cloth : public Entity
{
public:
    Cloth(Eigen::Vector3f positioin, Eigen::Vector3f rotation, Eigen::Vector3f scale);

    void Load_Texture(std::string filePath);

    void Update(float deltaTime);

private:
    //float m_Size;

    //std::vector<unsigned int> m_Edges;
    //std::vector<

    void Calc_Vertex(std::vector<Eigen::Vector3f> &vertices, std::vector<Eigen::Vector3f> &normals, std::vector<Eigen::Vector2f> &texcoords, std::vector<size_t> &indices);


        
};
