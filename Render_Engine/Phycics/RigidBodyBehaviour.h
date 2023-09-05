#pragma once

#include <eigen3/Eigen/Eigen>

#include "libexport.h"

#include "Behaviour.h"
#include "UniformBuffer.h"
#include "ShaderStorageBuffer.h"


namespace PhyE
{

    class DLLAPI RigidBodyBehaviour : public Behaviour
    {
    private:
        Shader* m_ComputeCollisionShader;

        Eigen::Matrix4f I_ref; // the reference to the moment of inertia
        Eigen::Matrix4f R; // entity's rotation

        bool launched = false; // shell we begin to simulate?

        ShaderStorageBuffer* OutputCollisionNumber; // binging id = 1
        ShaderStorageBuffer* OutputSumVec; // binding id = 2
        ShaderStorageBuffer* InputParamBuffer; // binding id = 3 
        ShaderStorageBuffer* VerticesBuffer; // point to the GPU buffer store the vertices ,binding id = 4
        ShaderStorageBuffer* InputR; // binding id = 5

        size_t total_Length = 0; // the number of entity's vertices

        struct InputParam {
        Eigen::Vector4f position;
        Eigen::Vector4f P;
        Eigen::Vector4f V;
        Eigen::Vector4f W;
        Eigen::Vector4f N;
        } InputParam;

    public:
        RigidBodyBehaviour(void* transform, float restitution, float friction, float gravity = -9.8f);

        void Start();
        void Update(float deltaTime);

        ~RigidBodyBehaviour() {
            if (m_ComputeCollisionShader)
                delete m_ComputeCollisionShader;
            if (VerticesBuffer)
                delete VerticesBuffer;
            if (InputParamBuffer)
                delete InputParamBuffer;
            if (InputR)
                delete InputR;
            if (OutputCollisionNumber)
                delete OutputCollisionNumber;
            if (OutputSumVec)
                delete OutputSumVec;
        }

    private:
        void Collision_Impulse(Eigen::Vector3f P, Eigen::Vector3f N);
        
        Eigen::Matrix4f Get_Cross_Matrix(Eigen::Vector3f a);

        Eigen::Quaternion<float> Add(Eigen::Quaternion<float> a, Eigen::Quaternion<float> b);

        Eigen::Vector3f Mat4f_Multiply_Vec3(Eigen::Matrix4f mat, Eigen::Vector3f vec);
        
    
    };
}
