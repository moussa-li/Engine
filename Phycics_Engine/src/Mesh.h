#pragma once

#include "libexport.h"

#include <eigen3/Eigen/Eigen>

namespace PhyE
{
    struct Vertex
    {
        Eigen::Vector3f Position;
        Eigen::Vector3f Normal;
    };
    class DLLAPI Mesh
    {
    private:
        std::vector<Vertex> m_Vertices;
        std::vector<>
    };
}
