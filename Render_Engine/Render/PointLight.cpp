#include "PointLight.h"

void PointLight::Draw()
{
    Mesh* mesh = m_Meshes[0];
    Transform transform = m_Transform;
    m_Shader->Bind();

    m_Shader->SetUniformMat4f("model", transform.GetMatrix());

    mesh->VAO->Bind();
    mesh->EBO->Bind();
    glDrawElements(GL_TRIANGLES, mesh->EBO->getCount(), GL_UNSIGNED_INT, 0);
    glDepthMask(GL_TRUE);
    mesh->VAO->Unbind();
    mesh->EBO->Unbind();
    m_Shader->Unbind();

}

