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
        RenderWork::instance().stop();
        UIWork::instance().stop();
        if (window)
        {
            window->terminate();
        }
    }

    void Application::exec()
    {
        RenderWork::instance().start();
        while (window->shouldClose() == false)
        {
            float currentFrame = window->getTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // GLFW event processing must remain on the main thread.
            glfwWaitEventsTimeout(0.016);
            cameraController->update(deltaTime);
        }

        RenderWork::instance().stop();
        UIWork::instance().stop();
        window->terminate();
    }
} // namespace EgLab::Platform