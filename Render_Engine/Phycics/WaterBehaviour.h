#pragma once

#include "libexport.h"

#include "Behaviour.h"

namespace PhyE
{

    class DLLAPI WaterBehaviour : public Behaviour
    {
    private:
        int m_Size = 100;
        float rate = 0.005f;
        float gamma = 0.002f;
        float damping = 0.996f;
        float bottom = -2.0f;
        float dt = 0.002f;
        //std::vector<Eigen::Vector3f> old_Vertices;
        //std::vector<Eigen::Vector3f> new_Vertices;
        std::vector<float> old_h;
        std::vector<float> h;
        std::vector<float> new_h;

        static Shader* WaterWaveShader;
        static ShaderStorageBuffer* old_hBuffer;
        static ShaderStorageBuffer* hBuffer;
        static ShaderStorageBuffer* new_hBuffer;
        
    public:
        WaterBehaviour(void *transform, int size);

        void Start();
        void Update(float deltaTime);

    private:
        void Shallow_Wave();
        
    };
}
