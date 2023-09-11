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
        std::vector<Eigen::Vector3f> vertices;
        std::vector<Eigen::Vector3f> normals;
        std::vector<Eigen::Vector2f> texcoords;
        std::vector<size_t> indices;
        Calc_Vertex(vertices, normals, texcoords, indices);
        Mesh* mesh = new Mesh(vertices, normals, texcoords, indices, {});
        m_Meshes.push_back(mesh);
	}

    Cube(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale,  float size, std::string ShaderPath)
        : Entity(position, rotation, scale, ShaderPath),
        Size(size) 
    {
        std::vector<Eigen::Vector3f> vertices;
        std::vector<Eigen::Vector3f> normals;
        std::vector<Eigen::Vector2f> texcoords;
        std::vector<size_t> indices;
        Calc_Vertex(vertices, normals, texcoords, indices);
        Mesh* mesh = new Mesh(vertices, normals, texcoords, indices, {});
        m_Meshes.push_back(mesh);
	}

    virtual void Calc_Vertex(std::vector<Eigen::Vector3f> &vertices, std::vector<Eigen::Vector3f> &normals, std::vector<Eigen::Vector2f> &texcoords, std::vector<size_t> &indices);
    virtual inline std::string Get_Type() { return "Cube"; }

};
