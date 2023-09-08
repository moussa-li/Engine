#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "eigen3/Eigen/Eigen"

#include "libexport.h"

#include <vector>

#include "Mesh.h"




namespace PhyE
{

    class DLLAPI Behaviour
    {
    protected:
        Eigen::Vector3f V;
        Eigen::Vector3f W;

        float dt = 0.015f;

        float m_LinearDecay = 0.999f;				// for velocity decay
        float m_AngularDecay = 0.98f;
        float m_Restitution = 0.5f;					// for collision
        float m_Friction = 0.2f;
        float m_Gravity;

        float m_Mass;

        std::vector<void*> m_Meshes;
        void* m_Transform;

        GLFWwindow* m_Window;



    public:
        Behaviour( void *transform, float restitution, float friction, float gravity = -9.8f);
        Behaviour(void* transform, float gravity = -9.8);
        virtual ~Behaviour(){
        }
        virtual void Start();
        virtual void Update(float deltaTime);

        void Load_Mesh(std::vector<Mesh*> meshes);
        

        void Set_Mesh(std::vector<Mesh*>& meshes);

        void Set_Window(GLFWwindow* window);
            

    };
}
