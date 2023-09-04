#include "RigidBody.h"


RigidBody::RigidBody(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale)
    : Entity(position, rotation, scale)
{
    m_Behaviour = new PhyE::RigidBodyBehaviour(&m_Transform, 0.5f, 0.2f);
    if(m_Window)
        m_Behaviour->Set_Window(m_Window);
}

RigidBody::~RigidBody()
{

    // TODO : This position have a strange bugs, called _CrtlsvaildHeapPointer(block) but it will memory leak .
    /*if (m_Behaviour)
        delete m_Behaviour;*/
}

void RigidBody::Update(float deltaTime)
{
    m_Behaviour->Update(0.015f);
}

//void Entity::Load(const std::string& fileDir)
//{
//    Load_Model(fileDir);
//    m_Behaviour->Load_Mesh(m_Meshes);
//}
