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

extern const unsigned int WIDTH;
extern const unsigned int HEIGHT;

class Renderer
{
public:
    void Clear();

    void Draw() const;

    void Insert_Entity(Entity* entity);
    
    void Insert_Light(Light* light);

    void SetBackGroundColor(Eigen::Vector4f backgroundcolor);

    Renderer(Camera *camera);

private:

    std::vector<Entity*> m_Entities;

    std::vector<Light*> m_Light;

    Camera *m_Camera;

    Eigen::Vector4f m_BackgroundColor;

};
