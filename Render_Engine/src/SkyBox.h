#pragma once

#include "libexport.h"

#include "Cube.h"
#include "Camera.h"

class DLLAPI SkyBox : public Cube
{
public:
    SkyBox(Camera * camera, std::string ShaderPath = "res/shaders/SkyBox.shader", std::string TexturePath = "res/textures/skybox");
    virtual void Draw();
    virtual void Calc_Vertex(std::vector<Vertex>& vertex, std::vector<size_t>& indices);
private:
    Camera* m_Camera;
    
        
};
