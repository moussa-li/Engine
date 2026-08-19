#include "test_render.h"

#include "Common/StringLineIterator.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/OrbitCameraController.hpp"
#include "RenderEngine/Core/RenderConfigure.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"
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
    EgLab::Common::SharedPtr<EgLab::RE::Window> window;
    EgLab::Common::SharedPtr<EgLab::RE::Renderer> renderer;
    EgLab::Common::SharedPtr<EgLab::RE::Scene> scene;
    EgLab::Common::SharedPtr<EgLab::RE::Camera> camera;
    EgLab::Common::SharedPtr<EgLab::RE::CameraController> cameraController;

    EgLab::RE::DeltaTime lastFrame;
    EgLab::RE::DeltaTime deltaTime;

    SimpleApp() : window(EgLab::Common::makeShared<EgLab::RE::Window>(800, 600))
    {
        camera = EgLab::Common::makeShared<EgLab::RE::Camera>(
            800, 600, EgLab::RE::CoordType(0.0f, 0.0f, 10.0f));
        cameraController = EgLab::Common::makeShared<EgLab::RE::OrbitCameraController>();
        cameraController->setCamera(camera);
        window->setCameraController(cameraController);

        renderer = EgLab::Common::makeShared<EgLab::RE::Renderer>();
        scene = EgLab::Common::makeShared<EgLab::RE::Scene>();
        EgLab::RE::RenderViewport viewport;
        viewport.window = window;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = 800;
        viewport.height = 600;
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

    EgLab::RE::CoordType coord(0, 0, 0);
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> coords;
    coords.pushBack(coord);
    auto v = EgLab::Common::makeShared<EgLab::RE::RenderNode>();
    v->setNodes(EgLab::Common::move(coords));
    v->setup();
    EgLab::Common::SharedPtr<EgLab::RE::Shader> shader;
    EgLab::RE::ShaderLib::instance().getNodeShader(shader);
    app.scene->addPrimitive(shader, v);

    EgLab::RE::CoordType coord1(0.5, 0, 0);
    auto v1 = EgLab::Common::makeShared<EgLab::RE::Vertex>(coord1);
    app.exec();
}

TEST_F(TestRender, line)
{
    SimpleApp app;

    EgLab::RE::CoordType coord(0, 0, 0);
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> coords;
    coords.pushBack(coord);
    EgLab::RE::CoordType coord1(0.5, 0, 0);
    coords.pushBack(coord1);

    coords.pushBack(coord);
    coords.pushBack(coord1);

    auto v = EgLab::Common::makeShared<EgLab::RE::RenderNode>();
    v->setNodes(EgLab::Common::move(coords));
    v->setup();
    EgLab::Common::SharedPtr<EgLab::RE::Shader> shader;
    EgLab::RE::ShaderLib::instance().getNodeShader(shader);
    app.scene->addPrimitive(shader, v);

    auto l = EgLab::Common::makeShared<EgLab::RE::RenderLine>();
    l->setNodes(EgLab::Common::move(coords));
    EgLab::Common::DynamicArray<EgLab::RE::IdxType> idxs;
    idxs.pushBack(0);
    idxs.pushBack(1);
    l->setIndices(EgLab::Common::move(idxs));
    l->setup();

    EgLab::RE::ShaderLib::instance().getLineShader(shader);
    app.scene->addPrimitive(shader, l);

    app.exec();
}

TEST_F(TestRender, box)
{
    SimpleApp app;

    EgLab::Common::Vector3f pos(0, 0, 0);
    EgLab::Common::Vector3f rot(0, 0, 0);
    EgLab::Common::Vector3f scal(1, 1, 1);
    auto box = EgLab::Common::makeShared<EgLab::RE::Box>();

    // app.scene->addEntity(box);

    EgLab::RE::CoordType coord(0, 0, 0);
    auto v = EgLab::Common::makeShared<EgLab::RE::Vertex>(coord);
    // app.scene->addEntity(v);
    app.exec();
}

TEST_F(TestRender, ShaderLib)
{
    EgLab::Common::String buffer;
    EgLab::Common::SharedPtr<EgLab::RE::Shader> shader;
    EgLab::RE::ShaderLib::instance().getBasicShader(shader);

    EgLab::Common::StringLineIterator it(buffer);
    EgLab::Common::DynamicArray<EgLab::Common::String> lines;
    for (; it.hasNext(); ++it)
    {
        const char* str;
        size_t len;
        it.getString(str, len);
        lines.pushBack(EgLab::Common::String(str, len));
    }
}