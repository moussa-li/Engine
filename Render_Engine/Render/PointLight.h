#pragma once

#include <eigen3/Eigen/Eigen>

#include "libexport.h"

#include "Cube.h"
#include "Light.h"

// define parallel light
class DLLAPI PointLight : public Cube , public Light
{
public:
    // TODO: maybe need more parameters
    PointLight(
        Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale, float size,
        float constant,
        float linear,
        float quadratic,
    Eigen::Vector3f ambient,
    Eigen::Vector3f diffuse,
    Eigen::Vector3f specular) 
        :   Cube(position,rotation,scale,size),
            Entity(position, rotation, scale),
            Light(ambient,diffuse,specular),
            m_Constant(constant),m_Linear(linear),m_Quadratic(quadratic)
    {
        //m_Direction = direction;
    }

    PointLight(
        Eigen::Vector3f position,
        float constant,
        float linear,
        float quadratic,
        Eigen::Vector3f ambient,
        Eigen::Vector3f diffuse,
        Eigen::Vector3f specular)
        : Cube(position, Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(1, 1, 1), 1, "res/shaders/Light.shader"),
        Entity(position, Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(1, 1, 1), "res/shaders/Light.shader"),
        Light(ambient, diffuse, specular),
        m_Constant(constant), m_Linear(linear), m_Quadratic(quadratic) {
    }

    // TODO: don't use , it need rebuild , i want to write Direction Light firstly
    virtual void SetShader(Entity* entity)
    {
        entity->Get_Shader()->Bind();
        entity->Get_Shader()->SetUniform3f("pointLights.position", m_Transform.Position);
        entity->Get_Shader()->SetUniform1f("pointLights.constant", m_Constant);
        entity->Get_Shader()->SetUniform1f("pointLights.linear", m_Linear);
        entity->Get_Shader()->SetUniform1f("pointLights.quadratic", m_Quadratic);
        entity->Get_Shader()->SetUniform3f("pointLights.ambient", GetAmbient());
        entity->Get_Shader()->SetUniform3f("pointLights.diffuse", GetDiffuse());
        entity->Get_Shader()->SetUniform3f("pointLights.specular", GetSpecular());
        entity->Get_Shader()->Unbind();
    }

    void Draw();
    
private:
    float m_Constant;
    float m_Linear;
    float m_Quadratic;


};
