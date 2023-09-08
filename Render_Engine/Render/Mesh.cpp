#include "Mesh.h"


void Mesh::Insert_Texture(Texture* texture)
{
    m_Texture.push_back(texture);
}

void Mesh::Setup()
{
    VAO = new VertexArray();

    /* vertices  layout */
    VertexBufferLayout layout;

    /* vertices coordinates vbo */
    VBO = new VertexBuffer(&m_Vertices);
    layout.Push<float>(3, VBO);

    std::unordered_set<Eigen::Vector3f, Vector3fHash, Vector3fEqual> hash_Vertices;
    for (int i = 0; i < m_Vertices.size(); ++i)
        hash_Vertices.insert(m_Vertices[i]);
    if (Vertices)
        delete[] Vertices;
    Vertices = new float[hash_Vertices.size() * 3];
    //std::vector<Eigen::Vector3f> vertices;
    size_t index = 0;
    for (const auto& vec : hash_Vertices)
    {
        Vertices[index++] = vec.x();
        Vertices[index++] = vec.y();
        Vertices[index++] = vec.z();
        //vertices.push_back(vec);
    }
    Vertices_Length = index;
    /*std::sort(vertices.begin(), vertices.end(), [](const Eigen::Vector3f& a, const Eigen::Vector3f& b) {
        return a.norm() < b.norm();
    });*/

    /* vertices normal vbo */
    Normal_VBO = new VertexBuffer(&m_Normals);
    layout.Push<float>(3, Normal_VBO);

    /* vertices texture coordinates vbo */
    Texture_VBO = new VertexBuffer(&m_TexCoords);
    layout.Push<float>(2, Texture_VBO);

    VAO->AddBuffer(layout);

    EBO = new IndexBuffer(m_indices);
    VAO->Unbind();

}

void Mesh::Clear()
{
    if (VAO)
    {
        delete VAO;
        VAO = nullptr;
    }
    if (Normal_VAO)
    {
        delete Normal_VAO;
        Normal_VAO = nullptr;
    }
    if (Texture_VAO)
    {
        delete Texture_VAO;
        Texture_VAO = nullptr;
    }

    if (VBO)
    {
        delete VBO;
        VBO = nullptr;
    }
    if (Normal_VBO)
    {
        delete Normal_VBO;
        Normal_VBO = nullptr;
    }
    if (Texture_VBO)
    {
        delete Texture_VBO;
        Texture_VBO = nullptr;
    }

    if (EBO)
    {
        delete EBO;
        EBO = nullptr;
    }
    if (Vertices)
    {
        delete[] Vertices;
        Vertices = nullptr;
    }
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

void Mesh::Update()
{
    Clear();
    // TODO : calculate the normals
    Setup();
}
