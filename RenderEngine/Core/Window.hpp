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

        void deal() const;

        void setCameraController(Common::SharedPtr<CameraController>);

        GLFWwindow *getNative() const;

        void terminate();

    private:
        class Impl;
        Impl *_impl;
    };

} // namespace EgLab::RE