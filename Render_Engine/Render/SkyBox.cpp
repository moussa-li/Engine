#include <eigen3/Eigen/Eigen>

#include "SkyBox.h"

// TODO : SkyBox constructor function
SkyBox::SkyBox(Camera *camera, std::string ShaderPath, std::string TexturePath)
    : Entity(camera->Position, Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,1,1), ShaderPath), 
      Cube(camera->Position, Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,1,1),1),
      m_Camera(camera)
{

    Texture* texture = new Texture(TexturePath, "skybox");
    m_Meshes[0]->Insert_Texture(texture);
}

void SkyBox::Draw()
{

    m_Transform.SetPosition(m_Camera->Position);
    for (size_t i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->Draw(m_Shader,m_Transform);
    }
}


void SkyBox::Calc_Vertex(std::vector<Vertex>& vertex, std::vector<size_t>& indices)
{

}

