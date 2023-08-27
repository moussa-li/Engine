#pragma once

#include <eigen3/Eigen/Eigen>

#include "Entity.h"

#include "libexport.h"


class DLLAPI Light
{
private:
    //Eigen::Vector3f m_Direction;
    Eigen::Vector3f m_Ambient;
    Eigen::Vector3f m_Diffuse;
    Eigen::Vector3f m_Specular;
public:
    Light(//Eigen::Vector3f direction,
    Eigen::Vector3f ambient,
    Eigen::Vector3f diffuse,
    Eigen::Vector3f specular)
        :  m_Ambient(ambient), m_Diffuse(diffuse), m_Specular(specular)
    {}

    inline Eigen::Vector3f GetAmbient()  const  { return m_Ambient; }   
    inline Eigen::Vector3f GetDiffuse()  const  { return m_Diffuse; }   
    inline Eigen::Vector3f GetSpecular() const  { return m_Specular; }  

    virtual void SetShader(Entity* entity)
    {
        entity->Get_Shader()->Bind();
        entity->Get_Shader()->SetUniform3f("pointLights.ambient", GetAmbient());
        entity->Get_Shader()->SetUniform3f("pointLights.diffuse", GetDiffuse());
        entity->Get_Shader()->SetUniform3f("pointLights.specular", GetSpecular());
        entity->Get_Shader()->Unbind();
    }
    

};
