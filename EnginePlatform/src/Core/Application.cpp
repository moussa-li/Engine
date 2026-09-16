#include "Core/Application.hpp"

#include "Core/MainMenuBar.hpp"
#include "GLFW/glfw3.h"
#include "RenderEngine/Core/OrbitCameraController.hpp"
#include "RenderEngine/Core/RenderConfigure.hpp"
#include "Work/RenderWork.hpp"
#include "Work/UIWork.hpp"

namespace EgLab::Platform
{

    Application::Application() : window(EgLab::Common::makeShared<EgLab::RE::Window>(800, 800))
    {
        window->activeContext();
        camera = EgLab::Common::makeShared<EgLab::RE::Camera>(
            800, 800, EgLab::RE::CoordType(0.0f, 0.0f, 1000.0f));
        cameraController = EgLab::Common::makeShared<EgLab::RE::OrbitCameraController>();
        cameraController->setCamera(camera);
        window->setCameraController(cameraController);

        window->deactiveContext();

        renderer = EgLab::Common::makeShared<EgLab::RE::Renderer>();
        scene = EgLab::Common::makeShared<EgLab::RE::Scene>();
        EgLab::RE::RenderViewport viewport;
        viewport.window = window;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = 800;
        viewport.height = 800;
        renderer->addViewport(viewport);

        RenderWork::instance().bindWindow(window);
        RenderWork::instance().bindRenderer(renderer);
        RenderWork::instance().bindScene(scene);
        RenderWork::instance().bindCamera(camera);
        UIWork::instance().bindWindow(window);
    }

    Application::~Application()
    {
    }

    void Application::exec()
    {
        RenderWork::instance().start();
        while (window->shouldClose() == false)
        {
            float currentFrame = window->getTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Mouse, keyboard and scroll events are already dispatched by the
            // GLFW callbacks attached in Window::Impl::start(); do not block
            // the application thread with glfwWaitEvents() here.
            glfwWaitEvents();
            cameraController->update(deltaTime);

            // Main/application thread prepares data only and leaves all GL
            // draw preparation to the render worker's thread-owned context.
            // window->deal();
        }

        RenderWork::instance().stop();
        UIWork::instance().stop();
        window->terminate();
    }
} // namespace EgLab::Platform