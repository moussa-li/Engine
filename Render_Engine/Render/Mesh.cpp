#include "Mesh.h"


void Mesh::Insert_Texture(Texture* texture)
{
    m_Texture.push_back(texture);
}

void Mesh::Setup()
{
    VAO = new VertexArray();
    VBO = new VertexBuffer(m_Vertices);
    VertexBufferLayout layout;
    layout.Push<float>(3);
    layout.Push<float>(3);
    layout.Push<float>(2);
    VAO->AddBuffer(*VBO, layout);
    EBO = new IndexBuffer(m_indices);

    VAO->Unbind();

}

void Mesh::Draw(Shader *shader,Transform transform)
{
    shader->Bind();
    shader->SetUniformMat4f("model", transform.GetMatrix());

    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    unsigned int skyboxNr = 1;
    for (size_t i = 0; i < m_Texture.size(); ++i)
    {
        m_Texture[i]->Bind(i);
        std::string number = "";
        std::string type = m_Texture[i]->GetType();
        if (type == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (type == "texture_specular")
            number = std::to_string(specularNr++);
        else if (type == "texture_normal")
            number = std::to_string(normalNr++);
        else if (type == "texture_height")
            number = std::to_string(heightNr++);
        else if (type == "skybox")
        {
            shader->SetUniform1i("skybox", 0);
            glDepthMask(GL_FALSE);
        }
        if (number != "")
        {
            shader->SetUniform1i("isPureColor", 0);
            shader->SetUniform1i(("material." + type + number).c_str(), i);
        }
    }

    // TODO: set mesh color (i don,t know where to put)
    if (m_Texture.size() == 0)
    {
        shader->SetUniform1i("isPureColor", 1);
        shader->SetUniform4f("u_Color", 0.8369918f,  1.f,  0.8381667f,  1.f);
    }

    VAO->Bind();
    EBO->Bind();
    glDrawElements(GL_TRIANGLES, EBO->getCount(), GL_UNSIGNED_INT, 0);
    glDepthMask(GL_TRUE);
    VAO->Unbind();
    EBO->Unbind();
    shader->Unbind();
}
