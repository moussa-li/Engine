#pragma once

#include <iostream>
#include <iomanip>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <eigen3/Eigen/Eigen>

using namespace std;
void Print(glm::mat4 m);


void Print(glm::vec3 v);

void Print(Eigen::Matrix4f mat);
