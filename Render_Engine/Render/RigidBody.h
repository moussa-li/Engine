#pragma once

#include "libexport.h"

#include "Entity.h"

#include "RigidBodyBehaviour.h"

class DLLAPI RigidBody : public Entity
{
//private:
    //PhyE::RigidBodyBehaviour m_Behaviour;
public:
    RigidBody(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale);

    ~RigidBody();

    virtual void Update(float deltaTime);


    //virtual void Load(const std::string& fileDir);

};
