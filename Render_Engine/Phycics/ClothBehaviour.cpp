#include <omp.h>

#include "ClothBehaviour.h"


PhyE::ClothBehaviour::ClothBehaviour(void* transform, float gravity /*= -9.8f*/)
    :Behaviour(transform,gravity)
{

}

void PhyE::ClothBehaviour::Start()
{
    m_PBDStatices = new Shader("res/shaders/PBDStatistics.shader");
    m_PBDCompute = new Shader("res/shaders/PBDCompute.shader");
    Mesh* mesh = (Mesh*)m_Meshes[0];
    auto vertices = mesh->m_Vertices;

    VerticesV.resize(mesh->m_Vertices.size(),Eigen::Vector3f(0,0,0));
    
    for (int i = 0; i < (mesh->m_indices).size(); i += 3)
    {
        size_t index1 = mesh->m_indices[i + 0], index2 = mesh->m_indices[i + 1];
        if (index1 > index2)std::swap(index1, index2);
        edges.insert({ index1,index2 });

        index1 = mesh->m_indices[i + 1], index2 = mesh->m_indices[i + 2];
        if (index1 > index2)std::swap(index1, index2);
        edges.insert({ index1,index2 });

        index1 = mesh->m_indices[i + 2], index2 = mesh->m_indices[i + 0];
        if (index1 > index2)std::swap(index1, index2);
        edges.insert({ index1,index2 });
    }

    for(auto it = edges.begin(); it != edges.end(); ++it)
    {
        m_EdgesLength.push_back(((mesh->m_Vertices)[it->first] - (mesh->m_Vertices)[it->second]).norm());

        m_Edges.push_back(it->first);
        m_Edges.push_back(it->second);
    }

    /* apply for stroage buffer */
    InputEdges = new ShaderStorageBuffer(m_Edges.size() * sizeof(size_t),1);
    InputEdges->InputData(m_Edges.data(), m_Edges.size() * sizeof(size_t));

    InputVertices = new ShaderStorageBuffer(mesh->Vertices_Length * sizeof(float) * 3, 2);

    InputEdgesL = new ShaderStorageBuffer(m_Edges.size() / 2 * sizeof(float), 3);
    InputEdgesL->InputData(m_EdgesLength.data(), m_EdgesLength.size() * sizeof(float));

    OutputSumX = new ShaderStorageBuffer(mesh->Vertices_Length * sizeof(int), 4);

    OutputSumN = new ShaderStorageBuffer(mesh->m_Vertices.size() * sizeof(unsigned int), 5);

    OutputVerticesV = new ShaderStorageBuffer(mesh->Vertices_Length * sizeof(float) * 3, 6);
    
}

void PhyE::ClothBehaviour::Update(float deltaTime)
{
    Mesh* mesh = (Mesh*)m_Meshes[0];
    std::vector<Eigen::Vector3f> X = mesh->m_Vertices;

    Eigen::Vector3f Accelerate = Eigen::Vector3f(0, m_Gravity, 0);

    for (size_t i = 0; i < X.size(); ++i)
    {
        if (i == 0 || i == 20) continue;

        VerticesV[i] += Accelerate * dt;
        VerticesV[i] *= damping;
        X[i] += VerticesV[i] * dt;
    }

    mesh->m_Vertices = X;
    InputVertices->InputData(X[0].data(), X.size() * sizeof(float)* 3);
    OutputVerticesV->InputData(VerticesV[0].data(), VerticesV.size() * sizeof(float) * 3);

    for (int i = 0; i < 32; ++i)
        Strain_Limiting();
    InputVertices->OutputData(mesh->m_Vertices[0].data());
    OutputVerticesV->OutputData(VerticesV[0].data());

    // TODO : collision with ball
    Collision_Handling();

    // TODO : re-calculate the normal
    
    mesh->Update();

}

void PhyE::ClothBehaviour::Strain_Limiting()
{
    Mesh* mesh = (Mesh*)m_Meshes[0];
    auto &vertices = mesh->m_Vertices;

    /* PBD here */
    
    size_t index = 0;

    m_PBDStatices->Bind();

    m_PBDStatices->SetUniform1i("max", m_Edges.size()/2);

    int group_x = (m_Edges.size()/2  + 1024) / 1024;
    GLCall(glDispatchCompute(group_x, 1, 1));
    GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BUFFER));

    m_PBDStatices->Unbind();
    

    m_PBDCompute->Bind();

    m_PBDCompute->SetUniform1f("dt", dt);
    m_PBDCompute->SetUniform1i("max", vertices.size());

    group_x = (vertices.size() + 1024) / 1024;
    GLCall(glDispatchCompute(group_x, 1, 1));
    GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BUFFER));

    
    m_PBDCompute->Unbind();
    
    //mesh->m_Vertices = vertices;

}

void PhyE::ClothBehaviour::Collision_Handling()
{

}
