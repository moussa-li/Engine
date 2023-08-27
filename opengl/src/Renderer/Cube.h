#pragma once
#include "Entity.h"
class Cube : public Entity
{
private:
	float Size;
public:
	Cube(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale, const std::string& Shader_filepath, float size)
        : Entity(position, rotation, scale, Shader_filepath),
        Size(size) {
	    }
};
