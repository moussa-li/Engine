#include "WaterBehaviour.h"



Shader* PhyE::WaterBehaviour::WaterWaveShader=nullptr;

ShaderStorageBuffer* PhyE::WaterBehaviour::old_hBuffer=nullptr;

ShaderStorageBuffer* PhyE::WaterBehaviour::hBuffer=nullptr;

ShaderStorageBuffer* PhyE::WaterBehaviour::new_hBuffer=nullptr;

PhyE::WaterBehaviour::WaterBehaviour(void *transform, int size)
    :Behaviour(transform), m_Size(size)
{
}

DLLAPI void PhyE::WaterBehaviour::Start()
{
    Mesh* mesh = (Mesh*)m_Meshes[0];
    std::vector<Eigen::Vector3f>& vertices = mesh->m_Vertices;
    h.resize(m_Size*m_Size,0);
    old_h.resize(m_Size*m_Size,0);
    new_h.resize(m_Size*m_Size,0);
    for (int i = 0; i < vertices.size(); ++i)
    {
        h[i] = vertices[i].y();
        //old_h[i / m_Size][i % m_Size] = 0;
    }
    if (!WaterWaveShader)
    {
        WaterWaveShader = new Shader("res/shaders/WaterWave.shader");
        WaterWaveShader->Bind();
        WaterWaveShader->SetUniform1f("damping", damping);
        //WaterWaveShader->SetUniform1f("damping", sm);
        WaterWaveShader->SetUniform1f("rate", rate);
        WaterWaveShader->SetUniform1i("size", m_Size);
        WaterWaveShader->Unbind();
    }
    if (!old_hBuffer)
        old_hBuffer = new ShaderStorageBuffer(vertices.size()  * sizeof(float), 31);
    old_hBuffer->InputData(old_h.data(), vertices.size() * sizeof(float));
    if (!hBuffer)
        hBuffer = new ShaderStorageBuffer(vertices.size()  * sizeof(float), 32);
    hBuffer->InputData(h.data(), vertices.size() * sizeof(float));
    if (!new_hBuffer)
        new_hBuffer = new ShaderStorageBuffer(vertices.size()  * sizeof(float), 33);
    new_hBuffer->InputData(new_h.data(), vertices.size() * sizeof(float));
}

DLLAPI void PhyE::WaterBehaviour::Update(float deltaTime)
{
    Mesh* mesh = (Mesh*)m_Meshes[0];
    std::vector<Eigen::Vector3f>& vertices = mesh->m_Vertices;
    for (int i = 0; i < vertices.size(); ++i)
    {
        h[i] = vertices[i].y();
    }

    if (glfwGetKey(m_Window, GLFW_KEY_R) == GLFW_PRESS)
    {
        std::srand(std::time(nullptr)); // use current time as seed for random generator
        int _i = std::rand()% m_Size;
        int _j = std::rand()% m_Size;
        int top_i = std::max(0, _i - 1);
        int bottom_i = std::min(m_Size-1, _i + 1);
        int left_j = std::max(0, _j - 1);
        int right_j = std::min(m_Size-1, _j + 1);
        float r = float(std::rand() % 4)/10 + 0.1f;
        float neighbor_r = r / 8.0f;
        h[_i*m_Size+_j] += r;
        h[top_i*m_Size+left_j] -= neighbor_r;
        h[top_i*m_Size+_j] -= neighbor_r;
        h[top_i*m_Size+right_j] -= neighbor_r;
        h[_i*m_Size+left_j] -= neighbor_r;
        h[_i*m_Size+right_j] -= neighbor_r;
        h[bottom_i*m_Size+left_j] -= neighbor_r;
        h[bottom_i*m_Size+_j] -= neighbor_r;
        h[bottom_i*m_Size+right_j] -= neighbor_r;
    }

    hBuffer->InputData(h.data(), vertices.size()  * sizeof(float));

    for (int l = 0; l < 8; l++)
        Shallow_Wave();
    new_hBuffer->OutputData(h.data());
    for (int i = 0; i < vertices.size(); ++i)
    {
        vertices[i].y() = h[i];
    }

    mesh->RecalculateNormals();
    mesh->Update();


}

DLLAPI void PhyE::WaterBehaviour::Shallow_Wave()
{
    Mesh* mesh = (Mesh*)m_Meshes[0];
    auto& vertices = mesh->m_Vertices;

    WaterWaveShader->Bind();
    int group_x = (100 + 32) / 32;
    GLCall(glDispatchCompute(group_x, group_x, 1));
    GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BUFFER));
    WaterWaveShader->Unbind();
}
