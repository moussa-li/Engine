#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "Camera.h"

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Entity.h"
#include "PointLight.h"
#include "SkyBox.h"

#include "libexport.h"

extern  unsigned int WIDTH;
extern  unsigned int HEIGHT;

class DLLAPI Renderer
{
public:
    void Clear();

    void Draw() const;

    void Insert_Entity(Entity* entity);
    
    void Insert_Light(Light* light);

    void SetBackGroundColor(Eigen::Vector4f backgroundcolor);

    void Update();

    Renderer(Camera *camera);

private:

    std::vector<Entity*> m_Entities;

    std::vector<Light*> m_Light;

    Camera *m_Camera;

    SkyBox *m_SkyBox;

    Eigen::Vector4f m_BackgroundColor;

};
