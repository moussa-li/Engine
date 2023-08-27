#pragma once
#include <eigen3/Eigen/Eigen>

struct Vertex {
    Eigen::Vector3f Position;
    Eigen::Vector3f Normal;
    Eigen::Vector2f TexCoords;
};
