#pragma once
#include "libexport.h"


namespace PhyE {

class DLLAPI Scene
{
public:
    Scene(float gravity = -9.81f, bool collision = true)
        : m_Gravity(gravity) , m_CollisionDetection(collision){}
    void Update();
private:
    float m_Gravity;
    bool m_CollisionDetection;
    
};
}
