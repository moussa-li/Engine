#include "Behaviour.h"
#include "Transform.h"

using namespace PhyE;
Behaviour::Behaviour( void *transform, float restitution, float friction, float gravity /*= -9.8f*/)
    :  m_Transform(transform), m_Restitution(restitution), m_Friction(friction), m_Gravity(gravity)
{
    m_Meshes = {};
}

void Behaviour::Start()
{
    

}

void Behaviour::Update(float deltaTime)
{
    /*Transform* transform = (Transform*)m_Transform;
    V += Eigen::Vector3f(0, m_Gravity * dt, 0);
    transform->Position += dt * V;*/
}

void Behaviour::Set_Mesh(std::vector<Mesh*>& meshes)
{
    meshes = {};
    for (size_t i = 0; i < m_Meshes.size(); ++i)
        meshes.push_back((Mesh*)m_Meshes[i]);
            

}

void Behaviour::Load_Mesh(std::vector<Mesh*> meshes)
{
    m_Meshes = {};
    for (size_t i = 0; i < meshes.size(); ++i)
        m_Meshes.push_back((void*)meshes[i]);
}

void Behaviour::Set_Window(GLFWwindow* window)
{
    m_Window = window;
}
