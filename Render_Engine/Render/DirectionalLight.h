#pragma once

#include <eigen3/Eigen/Eigen>

#include "Light.h"

class DirectionalLight : public Light
{
    Eigen::Vector3f m_Direction;
public:
    DirectionalLight(
        Eigen::Vector3f direction,
        Eigen::Vector3f ambient,
        Eigen::Vector3f diffuse,
        Eigen::Vector3f specular
    ) : Light(ambient, diffuse,specular) ,
        m_Direction(direction)
    {}

    inline Eigen::Vector3f GetDirection() const  { return m_Direction; }  

    virtual void SetShader(Entity* entity)
    {
        entity->Get_Shader()->Bind();
        entity->Get_Shader()->SetUniform3f("dirLight.direction", GetDirection());
        entity->Get_Shader()->SetUniform3f("dirLight.ambient", GetAmbient());
        entity->Get_Shader()->SetUniform3f("dirLight.diffuse", GetDiffuse());
        entity->Get_Shader()->SetUniform3f("dirLight.specular", GetSpecular());

        entity->Get_Shader()->Unbind();
    }
};
