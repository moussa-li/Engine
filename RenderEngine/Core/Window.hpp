#pragma once

#include "Common/SharedPtr.hpp"
#include "Core/Definites.hpp"
#include "RenderEngineAPI.hpp"

struct GLFWwindow;
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

        void maskEvent(bool);

        DeltaTime getTime() const;

        void start();

        void deal() const;

        void terminate();

        void setCameraController(Common::SharedPtr<CameraController>);

        GLFWwindow *getNative() const;

        void getFramebufferSize(unsigned int &width, unsigned int &height) const;

        void activeContext() const;

        void deactiveContext() const;

    private:
        class Impl;
        Impl *_impl;
    };

} // namespace EgLab::RE