#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "Reactor.h"

#include "Entity.h"
#include "Plane.h"
#include "DirectionalLight.h"

#include "Shader.h"
#include "Texture.h"
#include "Camera.h"

#include "Print.h"
#include "Log.h"

#include "libexport.h"

DLLAPI bool Windows_Init(unsigned int width, unsigned int height);
DLLAPI void Bind_Camera(Camera* camera);
DLLAPI void CalculateFrameRate();
DLLAPI void Windows_Start(Renderer * renderer);
DLLAPI void Windows_Terminate();

