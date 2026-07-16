#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Core/Definites.hpp"
#include "RenderEngineAPI.hpp"

namespace EgLab
{
    class RenderEngineAPI Window
    {
    public:
        Window();
        ~Window();
        Window(const unsigned int &width, const unsigned int &height);

        void setSize(const unsigned int &width, const unsigned int &height);

        bool shouldClose() const;

        DeltaTime getTime() const;

        void deal() const;

    private:
        class Impl;
        Impl *_impl;
    };

} // namespace EgLab