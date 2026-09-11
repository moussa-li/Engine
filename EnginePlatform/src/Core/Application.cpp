#include "Core/Application.hpp"

#include "RenderEngine/Core/OrbitCameraController.hpp"
#include "RenderEngine/Core/RenderConfigure.hpp"
#include "Work/RenderWork.hpp"
#include "Work/UIWork.hpp"

namespace EgLab::Platform
{

    Application::Application() : window(EgLab::Common::makeShared<EgLab::RE::Window>(800, 600))
    {
        camera = EgLab::Common::makeShared<EgLab::RE::Camera>(
            1920, 1080, EgLab::RE::CoordType(0.0f, 0.0f, 1000.0f));
        cameraController = EgLab::Common::makeShared<EgLab::RE::OrbitCameraController>();
        cameraController->setCamera(camera);
        window->setCameraController(cameraController);

        renderer = EgLab::Common::makeShared<EgLab::RE::Renderer>();
        scene = EgLab::Common::makeShared<EgLab::RE::Scene>();
        EgLab::RE::RenderViewport viewport;
        viewport.window = window;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = 1920;
        viewport.height = 1080;
        renderer->addViewport(viewport);

        RenderWork::instance().bindWindow(window);
        UIWork::instance().bindWindow(window);
    }

    Application::~Application()
    {
    }

    void Application::exec()
    {
        UIWork::instance().start();
        RenderWork::instance().start();
        while (window->shouldClose() == false)
        {
            float currentFrame = window->getTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            cameraController->update(deltaTime);
            scene->update(deltaTime);
            renderer->update(deltaTime);

            renderer->draw(scene, camera);

            UIWork::instance().renderFrame();
            window->deal();
        };

        RenderWork::instance().stop();
        UIWork::instance().stop();
        window->terminate();
    }
} // namespace EgLab::Platform