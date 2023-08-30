#include "RigidBodyBehaviour.h"
#include "Transform.h"
#include "Mesh.h"


PhyE::RigidBodyBehaviour::RigidBodyBehaviour(void* transform, float restitution, float friction, float gravity /*= -9.8f*/)
    : Behaviour(transform, restitution, friction)
{
}

void PhyE::RigidBodyBehaviour::Start()
{
    for (size_t i = 0; i < m_Meshes.size(); ++i)
    {
        std::vector<Vertex>  &vertices = ((Mesh*)m_Meshes[i])->m_Vertices;
        
        m_Mass = 0;
        float m = 1;
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            m_Mass += m;
            float diag = m * vertices[i].Position.squaredNorm();
            I_ref(0, 0) += diag;
            I_ref(1, 1) += diag;
            I_ref(2, 2) += diag;
            I_ref(0, 0) -= m * vertices[i].Position(0) * vertices[i].Position(0);
            I_ref(0, 1) -= m * vertices[i].Position(0) * vertices[i].Position(1);
            I_ref(0, 2) -= m * vertices[i].Position(0) * vertices[i].Position(2);
            I_ref(1, 0) -= m * vertices[i].Position(1) * vertices[i].Position(0);
            I_ref(1, 1) -= m * vertices[i].Position(1) * vertices[i].Position(1);
            I_ref(1, 2) -= m * vertices[i].Position(1) * vertices[i].Position(2);
            I_ref(2, 0) -= m * vertices[i].Position(2) * vertices[i].Position(0);
            I_ref(2, 1) -= m * vertices[i].Position(2) * vertices[i].Position(1);
            I_ref(2, 2) -= m * vertices[i].Position(2) * vertices[i].Position(2);
        }
    }
    I_ref(3, 3) = 1;
    R = ((Transform*)m_Transform)->Get_Rotation_Matrix();
}

void PhyE::RigidBodyBehaviour::Update(float deltaTime)
{
    if (glfwGetKey(m_Window, GLFW_KEY_R) == GLFW_PRESS)
    {
        ((Transform*)m_Transform)->SetPosition(Eigen::Vector3f(0, 0, 0));
        V = Eigen::Vector3f(0, 0, 0);
        W = Eigen::Vector3f(0, 0, 0);
        launched = false;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_L) == GLFW_PRESS)
    {
        launched = true;
        //V = Eigen::
    }
    if (launched)
    {
        /*for (size_t i = 0; i < m_Meshes.size(); ++i)
        {
            std::vector<Vertex> vertices = ((Mesh*)m_Meshes[i])->m_Vertices;
            for (size_t j = 0; j < vertices.size(); ++j)
            {
                if()
            }
        }*/
        Transform* transform = (Transform*)m_Transform;
        V += Eigen::Vector3f(0, m_Gravity * deltaTime, 0);
        //if(transform->Position)
        transform->Position += deltaTime * V;
    }
}

void PhyE::RigidBodyBehaviour::Collision_Impulse(Eigen::Vector3f P, Eigen::Vector3f N)
{

}

