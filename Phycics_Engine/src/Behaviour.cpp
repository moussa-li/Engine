#include "Behaviour.h"

using namespace PhyE;
Behaviour::Behaviour(void *mesh, float restitution, float friction, float gravity /*= -9.8f*/)
    : m_Mesh(mesh), m_Restitution(restitution), m_Friction(friction), m_Gravity(gravity)
{
}

void Behaviour::Start()
{

}

void Behaviour::Update()
{
    V += Eigen::Vector3f(0, m_Gravity * dt, 0);
}
