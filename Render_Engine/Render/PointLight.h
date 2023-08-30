#pragma once

#include <eigen3/Eigen/Eigen>

#include "Cube.h"
#include "Light.h"

// define parallel light
class PointLight : public Cube , public Light
{
public:
    // TODO: maybe need more parameters
    PointLight(
        Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale, float size,
    Eigen::Vector3f ambient,
    Eigen::Vector3f diffuse,
    Eigen::Vector3f specular) 
        :   Cube(position,rotation,scale,size),
            Light(ambient,diffuse,specular)
    {
        //m_Direction = direction;
    }

    // TODO: don't use , it need rebuild , i want to write Direction Light firstly
    virtual void SetShader(Entity* entity)
    {
        entity->Get_Shader()->Bind();
        entity->Get_Shader()->SetUniform3f("pointLights.ambient", GetAmbient());
        entity->Get_Shader()->SetUniform3f("pointLights.diffuse", GetDiffuse());
        entity->Get_Shader()->SetUniform3f("pointLights.specular", GetSpecular());

        entity->Get_Shader()->Unbind();
    }
    

};
