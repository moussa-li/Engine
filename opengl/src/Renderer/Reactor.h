#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include <eigen3/Eigen/Eigen>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


