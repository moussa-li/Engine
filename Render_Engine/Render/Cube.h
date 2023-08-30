#pragma once
#include "libexport.h"

#include "Entity.h"
class DLLAPI Cube : virtual public Entity
{
private:
	float Size;
public:
	Cube(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale,  float size)
        : Entity(position, rotation, scale),
        Size(size) 
    {
        std::vector<Vertex> vertex;
        std::vector<size_t> indices;
        Calc_Vertex(vertex, indices);
        Mesh* mesh = new Mesh(vertex, indices, {});
        m_Meshes.push_back(mesh);
	}
    virtual void Calc_Vertex(std::vector<Vertex> &vertex, std::vector<size_t> &indices);
    virtual inline std::string Get_Type() { return "Cube"; }

};
