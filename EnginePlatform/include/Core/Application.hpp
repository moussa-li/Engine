#pragma once

#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/Renderer.hpp"
#include "RenderEngine/Core/Scene.hpp"
#include "RenderEngine/Core/Window.hpp"

namespace EgLab::Platform
{

    class Application
    {
    public:
        EgLab::Common::SharedPtr<EgLab::RE::Window> window;
        EgLab::Common::SharedPtr<EgLab::RE::Renderer> renderer;
        EgLab::Common::SharedPtr<EgLab::RE::Scene> scene;
        EgLab::Common::SharedPtr<EgLab::RE::Camera> camera;
        EgLab::Common::SharedPtr<EgLab::RE::CameraController> cameraController;

        EgLab::RE::DeltaTime lastFrame;
        EgLab::RE::DeltaTime deltaTime;

    public:
        Application();

        ~Application();
        void exec();
    };

} // namespace EgLab::Platform