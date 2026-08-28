#include "test_render.h"

#include <GLFW/glfw3.h>

#include "Common/StringLineIterator.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/MeshPrimitiveCreator.hpp"
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
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window->getNative(), true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void execImGui()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static float mouseSensitivity = 1;
        static float zoomSpeed = 1;
        ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity, 1.f, 5.f, "%.2f");

        // 4. 显示当前值，方便调试
        ImGui::Text("Current Sensitivity: %.2f", mouseSensitivity);

        ImGui::SliderFloat("Zoom Speed", &zoomSpeed, 1.1f, 5.f, "%.2f");

        ImGui::Text("Current Zoom Speed: %.2f", zoomSpeed);

        auto controller =
            EgLab::Common::dynamicSharedPtrCast<EgLab::RE::OrbitCameraController>(cameraController);
        controller->setPanSensitivity(mouseSensitivity * 0.001f);
        controller->setZoomSpeed(zoomSpeed);

        // 渲染 UI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ImGuiIO &io = ImGui::GetIO();
        window->maskEvent(io.WantCaptureMouse);
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

            execImGui();
            window->deal();
        };

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        window->terminate();
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

TEST_F(TestRender, face)
{
    SimpleApp app;

    EgLab::RE::CoordType coord(0, 0, 0);
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> coords;
    coords.pushBack(coord);
    EgLab::RE::CoordType coord1(0.5, 0, 0);
    EgLab::RE::CoordType coord2(0.5, 0.5, 0);
    coords.pushBack(coord1);

    coords.pushBack(coord);
    coords.pushBack(coord1);
    coords.pushBack(coord2);

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

    coords.pushBack(coord);
    coords.pushBack(coord1);
    coords.pushBack(coord2);

    auto f = EgLab::Common::makeShared<EgLab::RE::RenderFace>();
    f->setNodes(EgLab::Common::move(coords));
    idxs.pushBack(0);
    idxs.pushBack(1);
    idxs.pushBack(2);
    f->setIndices(EgLab::Common::move(idxs));
    f->setup();

    EgLab::RE::ShaderLib::instance().getFaceShader(shader);
    app.scene->addPrimitive(shader, f);

    app.exec();
}

TEST_F(TestRender, trimesh)
{
    SimpleApp app;
    EgLab::Common::SharedPtr<EgLab::ME::Mesh> mesh = EgLab::Common::makeShared<EgLab::ME::Mesh>();

    for (int i = 0; i < 3; i++)
    {
        EgLab::ME::Node n;
        n.setId(i + 1);
        EgLab::ME::CoordType c(i % 2, i % 3, 0);
        n.setXYZ(c);
        mesh->addNode(EgLab::Common::move(n));
    }

    EgLab::ME::Elem e;
    e.setId(1);
    e.setType(EgLab::ME::ElemType::Tri3);
    e.setNode(0, 1);
    e.setNode(1, 2);
    e.setNode(2, 3);
    mesh->addElem(EgLab::Common::move(e));

    EgLab::ME::MeshIterator meshIt(*mesh);

    int i = 1;
    do
    {
        auto &n = meshIt.currentNode();
        EXPECT_EQ(n.getId(), i);
        i++;
    } while (meshIt.nextNode());

    auto &elem = meshIt.currentElem();
    EXPECT_EQ(elem.getId(), 1);

    EgLab::RE::MeshPrimitiveCreator creator(mesh);
    auto nodePrimitive = creator.getPrimitive<EgLab::RE::RenderNode>();
    auto linePrimitive = creator.getPrimitive<EgLab::RE::RenderLine>();
    auto facePrimitive = creator.getPrimitive<EgLab::RE::RenderFace>();

    EgLab::Common::SharedPtr<EgLab::RE::Shader> shader;
    EgLab::RE::ShaderLib::instance().getNodeShader(shader);
    app.scene->addPrimitive(shader, nodePrimitive);

    EgLab::RE::ShaderLib::instance().getLineShader(shader);
    app.scene->addPrimitive(shader, linePrimitive);

    EgLab::RE::ShaderLib::instance().getFaceShader(shader);
    app.scene->addPrimitive(shader, facePrimitive);

    app.exec();
}

TEST_F(TestRender, mesh)
{
    SimpleApp app;
    EgLab::Common::SharedPtr<EgLab::ME::Mesh> mesh = EgLab::Common::makeShared<EgLab::ME::Mesh>();

    int elemId[1] = {1};
    int nodeId[2][4] = {{1, 2, 3, 4}, {1, 3, 2, 5}};
    double nodeVec[5][3] = {
        {0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {0.5, 0.5, 1}, {0.5, 0.5, -1},
    };

    for (int i = 0; i < 5; i++)
    {
        EgLab::ME::Node n;
        n.setId(i + 1);
        EgLab::ME::CoordType c(nodeVec[i][0], nodeVec[i][1], nodeVec[i][2]);
        n.setXYZ(c);
        mesh->addNode(EgLab::Common::move(n));
    }

    for (int i = 0; i < 2; i++)
    {
        EgLab::ME::Elem e;
        e.setId(i + 1);
        e.setType(EgLab::ME::ElemType::Tet4);
        for (int j = 0; j < 4; j++)
        {
            e.setNode(j, nodeId[i][j]);
        }
        mesh->addElem(EgLab::Common::move(e));
    }

    EgLab::ME::MeshIterator meshIt(*mesh);

    int i = 1;
    do
    {
        auto &n = meshIt.currentNode();
        EXPECT_EQ(n.getId(), i);
        i++;
    } while (meshIt.nextNode());

    auto &elem = meshIt.currentElem();
    EXPECT_EQ(elem.getId(), 1);

    EgLab::RE::MeshPrimitiveCreator creator(mesh);
    auto nodePrimitive = creator.getPrimitive<EgLab::RE::RenderNode>();
    auto linePrimitive = creator.getPrimitive<EgLab::RE::RenderLine>();
    auto facePrimitive = creator.getPrimitive<EgLab::RE::RenderFace>();

    EgLab::Common::SharedPtr<EgLab::RE::Shader> shader;
    EgLab::RE::ShaderLib::instance().getNodeShader(shader);
    app.scene->addPrimitive(shader, nodePrimitive);

    EgLab::RE::ShaderLib::instance().getLineShader(shader);
    app.scene->addPrimitive(shader, linePrimitive);

    EgLab::RE::ShaderLib::instance().getFaceShader(shader);
    app.scene->addPrimitive(shader, facePrimitive);

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
        const char *str;
        size_t len;
        it.getString(str, len);
        lines.pushBack(EgLab::Common::String(str, len));
    }
}