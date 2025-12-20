#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "RenderEngineAPI.hpp"

namespace EgLab
{
    class Renderer;

    class RenderEngineAPI Window
    {
    public:
        Window();
        Window(const unsigned int &width, const unsigned int &height);

        void setSize(const unsigned int &width, const unsigned int &height);

        void exec();

        void setRenderer(Renderer *renderer);

    private:
        GLFWwindow *_window;

        Renderer *_renderer;

        unsigned int _width;
        unsigned int _height;

        float lastFrame{0.0f};
        float deltaTime{0.0f};
    };

} // namespace EgLab