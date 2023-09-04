#include "Collision.h"

float Signed_Distance_Plane(Eigen::Vector3f P, Eigen::Vector3f N, Eigen::Vector3f X)
{
    Eigen::Vector3f P_X = X - P;
    float result = P_X.dot(N);
    return result;
}

