#pragma once

#include "libexport.h"

#include <set>

#include "Behaviour.h"
#include "ShaderStorageBuffer.h"

namespace PhyE
{

    struct ComparePair {
        bool operator()(const std::pair<unsigned int, unsigned int>& a, const std::pair<unsigned int, unsigned int>& b) const {
            if (a.first != b.first)return a.first < b.first;
            else if (a.first == b.first)return a.second < b.second;
        };
    };

    typedef  std::set<std::pair<unsigned int, unsigned int>, ComparePair> EdgeSet;

    class DLLAPI ClothBehaviour : public Behaviour
    {
    private:
        std::vector<size_t> m_Edges;
        EdgeSet edges;
        std::vector<float> m_EdgesLength;

        float damping = 0.99f;
        float dt = 0.0333f;

        std::vector<Eigen::Vector3f> VerticesV;

        static int* a;

        static Shader* m_PBDStatices;

        static ShaderStorageBuffer* InputEdges;
        static ShaderStorageBuffer* InputVertices;
        static ShaderStorageBuffer* InputEdgesL;
        static ShaderStorageBuffer* OutputSumX;
        static ShaderStorageBuffer* OutputSumN;


        static Shader* m_PBDCompute;
        static ShaderStorageBuffer* OutputVertices;
        static ShaderStorageBuffer* InputSumX;
        static ShaderStorageBuffer* InputSumN;
        static ShaderStorageBuffer* OutputVerticesV;

    public:
        ClothBehaviour(void* transform, float gravity = -9.8f);

        void Start();
        void Update(float deltaTime);

        ~ClothBehaviour();
    private:
        void Strain_Limiting();
        void Collision_Handling();
    
    };


}
