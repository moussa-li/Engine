#pragma once

#include <eigen3/Eigen/Eigen>

#include "libexport.h"

/// <summary>
/// check whether the vertex is below or above the plane
/// if above > 0
/// else below < 0
/// else on the plane = 0
/// </summary>
/// <param name="P"> the point on the plane ,it's usually the position of the plane</param>
/// <param name="N"> the plane's normal </param>
/// <param name="X"> the vertex's position </param>
/// <returns></returns>
float Signed_Distance_Plane(Eigen::Vector3f P, Eigen::Vector3f N, Eigen::Vector3f X);


