#pragma once

#include <eigen3/Eigen/Eigen>

#include "libexport.h"

#include "Behaviour.h"


namespace PhyE
{

    class DLLAPI RigidBodyBehaviour : public Behaviour
    {
    private:
        Eigen::Matrix4f I_ref;
        Eigen::Matrix4f R;

        bool launched = false;
    public:
        RigidBodyBehaviour(void* transform, float restitution, float friction, float gravity = -9.8f);

        void Start();
        void Update(float deltaTime);

    private:
        void Collision_Impulse(Eigen::Vector3f P, Eigen::Vector3f N);
        
    
    };
}
