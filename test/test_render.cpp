#include "test_render.h"

#include "Common/StringLineIterator.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/OrbitCameraController.hpp"
#include "RenderEngine/Core/RenderConfigure.hpp"
#include "RenderEngine/Core/Renderer.hpp"
#include "RenderEngine/Core/Scene.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"
#include "RenderEngine/Core/Window.hpp"
#include "RenderEngine/Objects/Box.hpp"
#include "RenderEngine/Objects/Line.hpp"
#include "RenderEngine/Objects/Vertex.hpp"

class SimpleApp
{
public:
    EgLab::SharedPtr<EgLab::Window> window;
    EgLab::SharedPtr<EgLab::Renderer> renderer;
    EgLab::SharedPtr<EgLab::Scene> scene;
    EgLab::SharedPtr<EgLab::Camera> camera;
    EgLab::SharedPtr<EgLab::CameraController> cameraController;

    EgLab::DeltaTime lastFrame;
    EgLab::DeltaTime deltaTime;

    SimpleApp() : window(EgLab::makeShared<EgLab::Window>(300, 300))
    {
        camera = EgLab::makeShared<EgLab::Camera>(300, 300, EgLab::CoordType(0.0f, 0.0f, 10.0f));
        cameraController = EgLab::makeShared<EgLab::OrbitCameraController>();
        cameraController->setCamera(camera);
        window->setCameraController(cameraController);

        renderer = EgLab::makeShared<EgLab::Renderer>();
        scene = EgLab::makeShared<EgLab::Scene>();
        EgLab::RenderViewport viewport;
        viewport.window = window;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = 300;
        viewport.height = 300;
        renderer->addViewport(viewport);
    }

    ~SimpleApp() = default;
    void exec()
    {
        while (window->shouldClose() == false)
        {
            float currentFrame = window->getTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            cameraController->update(deltaTime);
            scene->update(deltaTime);
            renderer->update(deltaTime);

            renderer->draw(scene, camera);

            window->deal();
        };
    }
};

TEST_F(TestRender, window)
{
    SimpleApp app;
    app.exec();
}

TEST_F(TestRender, vertex)
{
    SimpleApp app;

    EgLab::CoordType coord(0, 0, 0);
    auto v = EgLab::makeShared<EgLab::Vertex>(coord);
    app.scene->addEntity(v);

    EgLab::CoordType coord1(0.5, 0, 0);
    auto v1 = EgLab::makeShared<EgLab::Vertex>(coord1);
    app.scene->addEntity(v1);
    app.exec();
}

TEST_F(TestRender, box)
{
    SimpleApp app;

    EgLab::Vector3f pos(0, 0, 0);
    EgLab::Vector3f rot(0, 0, 0);
    EgLab::Vector3f scal(1, 1, 1);
    auto box = EgLab::makeShared<EgLab::Box>(pos, rot, scal);

    app.scene->addEntity(box);

    EgLab::CoordType coord(0, 0, 0);
    auto v = EgLab::makeShared<EgLab::Vertex>(coord);
    app.scene->addEntity(v);
    app.exec();
}

TEST_F(TestRender, Line)
{
    SimpleApp app;

    EgLab::CoordType coord(0, 0, 0);
    auto v = EgLab::makeShared<EgLab::Vertex>(coord);
    app.scene->addEntity(v);

    EgLab::CoordType coord1(1, 0, 0);
    auto v1 = EgLab::makeShared<EgLab::Vertex>(coord1);
    app.scene->addEntity(v1);

    EgLab::DynamicArray<EgLab::CoordType> lineNodes;
    lineNodes.pushBack(coord);
    lineNodes.pushBack(coord1);
    auto l = EgLab::makeShared<EgLab::Line>(lineNodes);
    app.scene->addEntity(l);

    app.exec();
}

TEST_F(TestRender, ShaderLib)
{
    EgLab::String buffer;
    EgLab::ShaderLib::instance().getBasicShader(buffer);

    EgLab::StringLineIterator it(buffer);
    EgLab::DynamicArray<EgLab::String> lines;
    for (; it.hasNext(); ++it)
    {
        const char* str;
        size_t len;
        it.getString(str, len);
        lines.pushBack(EgLab::String(str, len));
    }
}