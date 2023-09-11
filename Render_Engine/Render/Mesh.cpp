#include "Mesh.h"

DLLAPI Shader * Mesh::RecalculateNormalsShader = nullptr;

DLLAPI ShaderStorageBuffer* Mesh::VerticesBuffer = nullptr;
DLLAPI ShaderStorageBuffer* Mesh::IndicesBuffer = nullptr;
DLLAPI ShaderStorageBuffer* Mesh::NormalsBuffer = nullptr;

void Mesh::Insert_Texture(Texture* texture)
{
    m_Texture.push_back(texture);
}

DLLAPI void Mesh::Setup()
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

    if(!RecalculateNormalsShader)
        RecalculateNormalsShader = new Shader("res/shaders/RecalculateNormals.shader");

    if(!IndicesBuffer)
        IndicesBuffer = new ShaderStorageBuffer(m_indices.size() * sizeof(unsigned int), 21);
    if(!VerticesBuffer)
        VerticesBuffer = new ShaderStorageBuffer(m_Vertices.size() * 3 * sizeof(float), 22);
    if(!NormalsBuffer)
        NormalsBuffer = new ShaderStorageBuffer(m_Vertices.size() * 3 * sizeof(float), 23);

    /*if(!IndicesBuffer)
        IndicesBuffer = new ShaderStorageBuffer(m_indices.size() * sizeof(unsigned int), 21);
    if(!VerticesBuffer)
        VerticesBuffer = new ShaderStorageBuffer(m_Vertices.size() * 3 * sizeof(float), 22);
    if(!NormalsBuffer)
        NormalsBuffer = new ShaderStorageBuffer(m_Vertices.size() * 3 * sizeof(float), 23);*/
    

}

DLLAPI void Mesh::Clear()
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
    //Setup();


    VAO = new VertexArray();

    /* vertices  layout */
    VertexBufferLayout layout;

    /* vertices coordinates vbo */
    VBO = new VertexBuffer(&m_Vertices);
    layout.Push<float>(3, VBO);

    if (Vertices)
        delete[] Vertices;
    Vertices = new float[m_Vertices.size() * 3];
    //std::vector<Eigen::Vector3f> vertices;
    memcpy(Vertices, m_Vertices.data(), m_Vertices.size() * 3 * sizeof(float));
    size_t index = 0;
    Vertices_Length = m_Vertices.size() * 3;
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

DLLAPI void Mesh::RecalculateNormals()
{
    
    
    VerticesBuffer->InputData(m_Vertices.data(), m_Vertices.size() * 3 * sizeof(float));
    NormalsBuffer->InputData(m_Normals.data(), m_Normals.size() * 3 * sizeof(float));
    IndicesBuffer->InputData(m_indices.data(), m_indices.size() * sizeof(unsigned int));
    RecalculateNormalsShader->Bind();
    int group_x = (m_indices.size()/3 + 1024) / 1024;
    RecalculateNormalsShader->SetUniform1i("max", m_indices.size()/3);
    GLCall(glDispatchCompute(group_x, 1, 1));
    GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BUFFER));
    NormalsBuffer->OutputData(m_Normals.data());

    RecalculateNormalsShader->Unbind();

    /*for (int i = 0; i < m_indices.size(); i+=3)
    {
        Eigen::Vector3f p1 = m_Vertices[m_indices[i+0]];
        Eigen::Vector3f p2 = m_Vertices[m_indices[i+1]];
        Eigen::Vector3f p3 = m_Vertices[m_indices[i+2]];

        Eigen::Vector3f normal = (p3 - p1).cross(p2 - p1);
        m_Normals[m_indices[i + 0]] = normal;
        m_Normals[m_indices[i + 1]] = normal;
        m_Normals[m_indices[i + 2]] = normal;
    }*/
}

Mesh::~Mesh()
{
        if (VAO)
        {
            delete VAO;
        }
        if (Normal_VAO)
        {
            delete Normal_VAO;
        }
        if (Texture_VAO)
        {
            delete Texture_VAO;
        }

        if (VBO)
        {
            delete VBO;
        }
        if (Normal_VBO)
        {
            delete Normal_VBO;
        }
        if (Texture_VBO)
        {
            delete Texture_VBO;
        }

        if (EBO)
        {
            delete EBO;
        }
        if (Vertices)
        {
            delete[] Vertices;
        }
        if (RecalculateNormalsShader)
        {
            delete RecalculateNormalsShader;
            RecalculateNormalsShader = nullptr;
        }
        if (IndicesBuffer)
        {
            delete IndicesBuffer;
            IndicesBuffer = nullptr;
        }
        if (VerticesBuffer)
        {
            delete VerticesBuffer;
            VerticesBuffer = nullptr;
        }
        if (NormalsBuffer)
        {
            delete NormalsBuffer;
            NormalsBuffer = nullptr;
        }
    }
