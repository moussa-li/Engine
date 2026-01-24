#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Common/UniquePtr.hpp"
#include "Core/Definites.hpp"
#include "RenderEngineAPI.hpp"

namespace EgLab
{
    class Renderer;

    class RenderEngineAPI Window
    {
    public:
        Window();
        ~Window();
        Window(const unsigned int &width, const unsigned int &height);

        void setSize(const unsigned int &width, const unsigned int &height);

        bool shouldClose() const;
        // void exec();
        DeltaTime getTime() const;

        void setRenderer(Renderer *renderer);

    private:
        class Impl;
        Impl *_impl;

        // UniquePtr<Renderer> _renderer;
    };

} // namespace EgLab