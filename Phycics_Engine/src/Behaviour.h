#pragma once
#include "libexport.h"

#include "eigen3/Eigen/Eigen"

namespace PhyE
{

    class DLLAPI Behaviour
    {
    private:
        Eigen::Vector3f V;
        Eigen::Vector3f W;

        float dt = 0.015f;

        float m_LinearDecay = 0.999f;				// for velocity decay
        float m_AngularDecay = 0.98f;
        float m_Restitution;					// for collision
        float m_Friction;

        void* m_Mesh;


        float m_Gravity;
    public:
        Behaviour(void* mesh, float restitution, float friction, float gravity = -9.8f);
        void Start();
        void Update();

    };
}
