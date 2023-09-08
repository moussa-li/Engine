#include <omp.h>

#include "ClothBehaviour.h"


PhyE::ClothBehaviour::ClothBehaviour(void* transform, float gravity /*= -9.8f*/)
    :Behaviour(transform,gravity)
{

}

void PhyE::ClothBehaviour::Start()
{
    m_PBDStatices = new Shader("res/shaders/PBDStatistics.shader");
    Mesh* mesh = (Mesh*)m_Meshes[0];

    VerticesV.resize(mesh->m_Vertices.size(),Eigen::Vector3f(0,0,0));
    /*std::vector<std::pair<unsigned int, unsigned int>> edges((mesh->m_indices).size());
    for (int i = 0; i < (mesh->m_indices).size(); i+=3)
    {
        edges[i + 0].first = mesh->m_indices[i + 0];
        edges[i + 0].second = mesh->m_indices[i + 1];
        edges[i + 1].first = mesh->m_indices[i + 1];
        edges[i + 1].second = mesh->m_indices[i + 2];
        edges[i + 2].first = mesh->m_indices[i + 2];
        edges[i + 2].second = mesh->m_indices[i + 0];
    }
    for (int i = 0; i < edges.size(); ++i)
    {
        if (edges[i].first > edges[i].second)
        {
            std::swap(edges[i].first, edges[i].second);
        }
    }*/
    //std::sort(edges.begin(), edges.end());
    //size_t EdgesNumber = 0;
    //std::set<std::pair<unsigned int, unsigned int>, ComparePair> edges;
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

    InputEdges = new ShaderStorageBuffer(m_Edges.size() * sizeof(size_t),1);
    InputEdges->InputData(m_Edges.data(), m_Edges.size() * sizeof(size_t));
    InputVertices = new ShaderStorageBuffer(mesh->Vertices_Length * sizeof(float) * 3, 2);
    InputEdgesL = new ShaderStorageBuffer(m_Edges.size() / 2 * sizeof(float), 3);
    OutputSumX = new ShaderStorageBuffer(mesh->Vertices_Length * sizeof(int), 4);
    OutputSumN = new ShaderStorageBuffer(mesh->m_Vertices.size() * sizeof(unsigned int), 5);
    //OutputSumN->InputData(std::vector<size_t>(mesh->m_Vertices.size(),0).data(),mesh->m_Vertices.size() * sizeof(unsigned int));
    //std::sort(edges.begin(), edges.end(), []}
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

    for (int i = 0; i < 32; ++i)
        Strain_Limiting();

    Collision_Handling();
    
    mesh->Update();

}

void PhyE::ClothBehaviour::Strain_Limiting()
{
    Mesh* mesh = (Mesh*)m_Meshes[0];
    auto vertices = mesh->m_Vertices;


    /* PBD here */
    std::vector<Eigen::Vector3f> SumX(vertices.size(),Eigen::Vector3f(0,0,0));
    std::vector<Eigen::Vector3i> SumXi(vertices.size(),Eigen::Vector3i(0,0,0));
    std::vector<unsigned int> SumN(vertices.size(),0);

    size_t index = 0;

    m_PBDStatices->Bind();
    InputVertices->InputData(vertices[0].data(), vertices.size() * sizeof(float)* 3);
    InputEdgesL->InputData(m_EdgesLength.data(), m_EdgesLength.size() * sizeof(float));

    m_PBDStatices->SetUniform1i("max", m_Edges.size()/2);

    int group_x = (m_Edges.size()/2  + 1024) / 1024;
    GLCall(glDispatchCompute(group_x, 1, 1));
    GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BUFFER));

    OutputSumN->OutputData(&SumN[0]);
    OutputSumX->OutputData(&SumXi[0]);
    for (int i = 0; i < SumX.size(); i++)
    {
        SumX[i].x()= (float)SumXi[i].x()/10000000;
        SumX[i].y()= (float)SumXi[i].y()/10000000;
        SumX[i].z()= (float)SumXi[i].z()/10000000;
    }
    
    // TODO: it was tooooooooooo slow!!!! 2PFS LAMO
    for (int i = 0; i < vertices.size(); ++i)
    {
        if (i == 0 || i == 20) continue;
        VerticesV[i] += 1 / dt * ((0.2f * vertices[i] + SumX[i]) / (0.2f + SumN[i]) - vertices[i]);
        vertices[i] = (0.2f * vertices[i] + SumX[i]) / (0.2f + SumN[i]);
    }
    
    mesh->m_Vertices = vertices;

}

void PhyE::ClothBehaviour::Collision_Handling()
{

}
