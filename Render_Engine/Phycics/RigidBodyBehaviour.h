#pragma once

#include <eigen3/Eigen/Eigen>

#include "libexport.h"

#include "Behaviour.h"


namespace PhyE
{

    class DLLAPI RigidBodyBehaviour : public Behaviour
    {
    private:
        Shader* m_ComputeCollisionShader;

        Eigen::Matrix4f I_ref;
        Eigen::Matrix4f R;

        bool launched = false;

        //int collision_number = 0;
        GLuint ssbo;
        GLuint ssbo2;
        GLuint ssbo3;
        GLuint ssbo4;
        GLuint ssbo5;
        GLuint ssbo6;

        size_t total_Length = 0;

    public:
        RigidBodyBehaviour(void* transform, float restitution, float friction, float gravity = -9.8f);

        void Start();
        void Update(float deltaTime);

        ~RigidBodyBehaviour() {
            if (m_ComputeCollisionShader)
                delete m_ComputeCollisionShader;
        }

    private:
        void Collision_Impulse(Eigen::Vector3f P, Eigen::Vector3f N);
        
        Eigen::Matrix4f Get_Cross_Matrix(Eigen::Vector3f a);

        Eigen::Quaternion<float> Add(Eigen::Quaternion<float> a, Eigen::Quaternion<float> b);

        Eigen::Vector3f Mat4f_Multiply_Vec3(Eigen::Matrix4f mat, Eigen::Vector3f vec);
        
    
    };
}
