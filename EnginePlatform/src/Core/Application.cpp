#include "Core/Application.hpp"

#include <algorithm>

#include "Command/CommandManager.hpp"
#include "Core/IniManager.hpp"
#include "Core/MainMenuBar.hpp"
#include "GLFW/glfw3.h"
#include "RenderEngine/Core/OrbitCameraController.hpp"
#include "RenderEngine/Core/RenderConfigure.hpp"
#include "Work/RenderWork.hpp"
#include "Work/UIWork.hpp"

namespace EgLab::Platform
{

    Application::Application()
    {
        auto& ini = IniManager::instance();
        auto& cmdMgr = CommandManager::instance();
        ini.start();
        const auto width = static_cast<unsigned int>(std::max(1, ini.getInt("window.width", 800)));
        const auto height =
            static_cast<unsigned int>(std::max(1, ini.getInt("window.height", 800)));
        window = EgLab::Common::makeShared<EgLab::RE::Window>(width, height);
        window->setMaximized(ini.getBool("window.maximized", false));

        window->activeContext();
        camera = EgLab::Common::makeShared<EgLab::RE::Camera>(
            width, height, EgLab::RE::CoordType(0.0f, 0.0f, 1000.0f));
        camera->setMoveSensitivity(ini.getFloat("camera.moveSensitivity", 1.0f));
        camera->setRotateSensitivity(ini.getFloat("camera.rotateSensitivity", 1.0f));
        camera->setZoomSensitivity(ini.getFloat("camera.zoomSensitivity", 1.0f));
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
        viewport.width = static_cast<int>(width);
        viewport.height = static_cast<int>(height);
        renderer->addViewport(viewport);

        auto background = renderer->getBackGroundColor();
        background[0] = ini.getFloat("appearance.background.r", background[0]);
        background[1] = ini.getFloat("appearance.background.g", background[1]);
        background[2] = ini.getFloat("appearance.background.b", background[2]);
        background[3] = ini.getFloat("appearance.background.a", background[3]);
        renderer->setBackGroundColor(background);
        ini.setFloat("appearance.background.r", background[0]);
        ini.setFloat("appearance.background.g", background[1]);
        ini.setFloat("appearance.background.b", background[2]);
        ini.setFloat("appearance.background.a", background[3]);

        auto light = renderer->getLightSettings();
        light.ambient[0] = ini.getFloat("light.ambient.r", light.ambient[0]);
        light.ambient[1] = ini.getFloat("light.ambient.g", light.ambient[1]);
        light.ambient[2] = ini.getFloat("light.ambient.b", light.ambient[2]);
        light.diffuse[0] = ini.getFloat("light.diffuse.r", light.diffuse[0]);
        light.diffuse[1] = ini.getFloat("light.diffuse.g", light.diffuse[1]);
        light.diffuse[2] = ini.getFloat("light.diffuse.b", light.diffuse[2]);
        light.specular[0] = ini.getFloat("light.specular.r", light.specular[0]);
        light.specular[1] = ini.getFloat("light.specular.g", light.specular[1]);
        light.specular[2] = ini.getFloat("light.specular.b", light.specular[2]);
        light.shininess = ini.getFloat("light.shininess", light.shininess);
        renderer->setLightSettings(light);
        renderer->setRenderNodesEnabled(
            ini.getBool("mesh.renderNodes", renderer->isRenderNodesEnabled()));
        renderer->setRenderEdgesEnabled(
            ini.getBool("mesh.renderEdges", renderer->isRenderEdgesEnabled()));
        renderer->setRenderFacesEnabled(
            ini.getBool("mesh.renderFaces", renderer->isRenderFacesEnabled()));

        RenderWork::instance().bindWindow(window);
        RenderWork::instance().bindRenderer(renderer);
        RenderWork::instance().bindScene(scene);
        RenderWork::instance().bindCamera(camera);
        UIWork::instance().bindWindow(window);

        RenderWork::instance().setHighlightColor(Common::Vector4f(
            ini.getFloat("mesh.highlight.r", 1.0f), ini.getFloat("mesh.highlight.g", 0.75f),
            ini.getFloat("mesh.highlight.b", 0.7f), ini.getFloat("mesh.highlight.a", 0.3f)));
    }

    Application::~Application()
    {
        RenderWork::instance().stop();
        UIWork::instance().stop();
        if (window)
        {
            if (window->getNative())
            {
                unsigned int width = 0;
                unsigned int height = 0;
                window->getWindowSize(width, height);
                IniManager::instance().setInt("window.width", static_cast<int>(width));
                IniManager::instance().setInt("window.height", static_cast<int>(height));
                IniManager::instance().setBool("window.maximized", window->isMaximized());
            }
        }
        IniManager::instance().shutdown();
        if (window)
        {
            if (window->getNative())
            {
                window->activeContext();
                RenderWork::instance().releaseResources();
                scene = nullptr;
                renderer = nullptr;
                window->deactiveContext();
            }
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
        unsigned int width = 0;
        unsigned int height = 0;
        window->getWindowSize(width, height);
        IniManager::instance().setInt("window.width", static_cast<int>(width));
        IniManager::instance().setInt("window.height", static_cast<int>(height));
        IniManager::instance().setBool("window.maximized", window->isMaximized());
    }
} // namespace EgLab::Platform