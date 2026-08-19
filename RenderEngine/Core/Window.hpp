#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Common/SharedPtr.hpp"
#include "Core/Definites.hpp"
#include "RenderEngineAPI.hpp"

namespace EgLab::RE
{

    class CameraController;
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

        void setCameraController(Common::SharedPtr<CameraController>);

    private:
        class Impl;
        Impl *_impl;
    };

} // namespace EgLab::RE