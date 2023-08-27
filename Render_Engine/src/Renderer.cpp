#include "Renderer.h"
#include <iostream>


void Renderer::Clear()
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT);)
}


/// <summary>
/// draw every entities and set camera
/// </summary>
void Renderer::Draw() const
{
    for (size_t i = 0; i < m_Entities.size(); ++i)
    {
        m_Entities[i]->Draw();
    }
    for (size_t i = 0; i < m_Entities.size(); ++i)
    {
        Eigen::Matrix4f proj = Eigen::Matrix4f::Identity();
        proj = m_Camera->perspective(m_Camera->radians(m_Camera->Zoom), (float)WIDTH/HEIGHT, 0.1f, 100.0f);
        Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
        view = m_Camera->View();

        m_Entities[i]->Shader_Load_Camera(proj, view);
        for (size_t j = 0; j < m_Light.size(); ++j)
        {
             m_Light[j]->SetShader(m_Entities[i]);
        }
        m_Entities[i]->Get_Shader()->Bind();
        m_Entities[i]->Get_Shader()->SetUniform3f("viewPos", m_Camera->Position);
        m_Entities[i]->Get_Shader()->Unbind();
    }


}

/// <summary>
//  insert the entity which renderer need to render
/// </summary>
/// <param name="entity">  pointer to insert entity </param>
void Renderer::Insert_Entity(Entity* entity)
{
    if (entity)
        m_Entities.push_back(entity);
    else
        std::cout << "entity is nullptr , cannot insert to renderer!!!" << std::endl;
}

void Renderer::Insert_Light(Light* light)
{
    m_Light.push_back(light);
}

// set glfw background color
void Renderer::SetBackGroundColor(Eigen::Vector4f BackgroundColor)
{
    m_BackgroundColor = BackgroundColor;
    glClearColor(m_BackgroundColor(0),m_BackgroundColor(1),m_BackgroundColor(2),m_BackgroundColor(3));//È·¶¨ÑÕÉ«

}

Renderer::Renderer(Camera* camera)
{
    m_Camera = camera;
}
