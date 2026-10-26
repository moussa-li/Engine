#include "test_render.h"

#include <GLFW/glfw3.h>

#include <chrono>

#include "Common/StringLineIterator.hpp"
#include "MeshEngine/IO/GmshImporter.hpp"
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
#include "test_utils.hpp"

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
            800, 600, EgLab::RE::CoordType(0.0f, 0.0f, 1000.0f));
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

        window->start();
        window->activeContext();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window->getNative(), true);
        ImGui_ImplOpenGL3_Init("#version 330");

        window->deactiveContext();
    }

    void execImGui()
    {
        window->activeContext();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static float mouseSensitivity = 1;
        static float zoomSpeed = 1.1;
        static float orbitSensitivity = 1;
        ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity, 1.f, 5.f, "%.2f");

        // 4. 显示当前值，方便调试
        ImGui::Text("Current Sensitivity: %.2f", mouseSensitivity);

        ImGui::SliderFloat("Zoom Speed", &zoomSpeed, 1.1f, 5.f, "%.2f");

        ImGui::Text("Current Zoom Speed: %.2f", zoomSpeed);

        ImGui::SliderFloat("Orbit Sensitivity", &orbitSensitivity, 1.1f, 5.f, "%.2f");

        ImGui::Text("Current Orbit Sensitivity: %.2f", orbitSensitivity);

        auto controller =
            EgLab::Common::dynamicSharedPtrCast<EgLab::RE::OrbitCameraController>(cameraController);
        controller->setPanSensitivity(mouseSensitivity * 0.001f);
        controller->setZoomSpeed(zoomSpeed);
        controller->setOrbitSensitivity(orbitSensitivity * 0.01f);

        // 渲染 UI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ImGuiIO &io = ImGui::GetIO();
        window->maskEvent(io.WantCaptureMouse);

        window->deactiveContext();
    }

    ~SimpleApp() = default;
    void exec()
    {
        const auto startTime = std::chrono::steady_clock::now();
        while (window->shouldClose() == false &&
               std::chrono::steady_clock::now() - startTime < std::chrono::seconds(1))
        {
            float currentFrame = window->getTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            cameraController->update(deltaTime);
            scene->update(deltaTime);
            renderer->update(deltaTime);

            window->activeContext();
            renderer->draw(scene, camera);
            window->deactiveContext();

            execImGui();
            window->deal();
        };

        window->activeContext();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        window->deactiveContext();
        window->terminate();
    }
};

TEST_F(TestRender, window)
{
    // SimpleApp app;
    // app.exec();
}

TEST_F(TestRender, node_touch_uses_screen_space)
{
    EgLab::RE::RenderNode node;
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> nodes;
    nodes.pushBack(EgLab::RE::CoordType(0.0f, 0.0f, 0.0f));
    node.setNodes(EgLab::Common::move(nodes));

    EgLab::RE::PickContext context;
    context.width = 100;
    context.height = 100;
    context.view = EgLab::Common::Matrix4f::Identity();
    context.view[2][3] = -1000.0f;
    context.projection = EgLab::Common::Matrix4f::Identity();
    context.projection[2][2] = -0.0002f;
    context.projection[2][3] = -1.0f;

    context.x = 50.0f;
    context.y = 50.0f;
    EXPECT_EQ(node.touch(context), 0u);

    context.x = 58.0f;
    EXPECT_EQ(node.touch(context), static_cast<EgLab::RE::IdxType>(-1));
}

TEST_F(TestRender, node_touch_prefers_frontmost_node)
{
    EgLab::RE::RenderNode backNode;
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> backNodes;
    backNodes.pushBack(EgLab::RE::CoordType(0.0f, 0.0f, 0.0f));
    backNode.setNodes(EgLab::Common::move(backNodes));

    EgLab::RE::RenderNode frontNode;
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> frontNodes;
    frontNodes.pushBack(EgLab::RE::CoordType(0.0f, 0.0f, 1.0f));
    frontNode.setNodes(EgLab::Common::move(frontNodes));

    EgLab::RE::PickContext context;
    context.width = 100;
    context.height = 100;
    context.x = 50.0f;
    context.y = 50.0f;
    context.view = EgLab::Common::Matrix4f::Identity();
    context.view[2][3] = -1000.0f;
    context.projection = EgLab::Common::Matrix4f::Identity();
    context.projection[2][2] = -0.0002f;
    context.projection[2][3] = -1.0f;

    EXPECT_EQ(backNode.touch(context), 0u);
    EXPECT_EQ(frontNode.touch(context), 0u);
    EXPECT_EQ(backNode.touch(context), static_cast<EgLab::RE::IdxType>(-1));
}

TEST_F(TestRender, vertex)
{
    SimpleApp app;

    EgLab::RE::CoordType coord(0, 0, 0);
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> coords;
    coords.pushBack(coord);
    auto v = EgLab::Common::makeShared<EgLab::RE::RenderNode>();
    v->setNodes(EgLab::Common::move(coords));
    app.window->activeContext();
    v->setup();

    EgLab::Common::SharedPtr<EgLab::RE::Shader> shader;
    EgLab::RE::ShaderLib::instance().getNodeShader(shader);
    app.scene->addPrimitive(shader, v);
    app.window->deactiveContext();

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

    app.window->activeContext();
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

    app.window->deactiveContext();
    app.exec();
}

TEST_F(TestRender, renderLineTouch)
{
    EgLab::RE::RenderLine line;
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> vertices;
    vertices.pushBack(EgLab::RE::CoordType(-0.5f, 0.0f, -0.5f));
    vertices.pushBack(EgLab::RE::CoordType(0.5f, 0.0f, -0.5f));
    line.setNodes(EgLab::Common::move(vertices));

    EgLab::Common::DynamicArray<EgLab::RE::IdxType> indices;
    indices.pushBack(0);
    indices.pushBack(1);
    line.setIndices(EgLab::Common::move(indices));

    EgLab::RE::PickContext context;
    context.width = 200;
    context.height = 200;
    context.x = 100.0f;
    context.y = 100.0f;
    context.view = EgLab::Common::Matrix4f::Identity();
    context.projection = EgLab::Common::Matrix4f::Identity();
    EXPECT_EQ(line.touch(context), 0u);

    EgLab::RE::PickContext outsideContext = context;
    outsideContext.x = 100.0f;
    outsideContext.y = 120.0f;
    EXPECT_EQ(line.touch(outsideContext), static_cast<EgLab::RE::IdxType>(-1));
}

TEST_F(TestRender, renderFaceTouch)
{
    EgLab::RE::RenderFace face;
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> vertices;
    vertices.pushBack(EgLab::RE::CoordType(-0.5f, -0.5f, 0.5f));
    vertices.pushBack(EgLab::RE::CoordType(0.5f, -0.5f, 0.5f));
    vertices.pushBack(EgLab::RE::CoordType(0.0f, 0.5f, 0.5f));
    face.setNodes(EgLab::Common::move(vertices));

    EgLab::Common::DynamicArray<EgLab::RE::IdxType> indices;
    indices.pushBack(0);
    indices.pushBack(1);
    indices.pushBack(2);
    face.setIndices(EgLab::Common::move(indices));

    EgLab::RE::PickContext context;
    context.width = 200;
    context.height = 200;
    context.x = 100.0f;
    context.y = 100.0f;
    context.view = EgLab::Common::Matrix4f::Identity();
    context.projection = EgLab::Common::Matrix4f::Identity();
    EXPECT_EQ(face.touch(context), 0u);

    EgLab::RE::PickContext outsideContext = context;
    outsideContext.x = 10.0f;
    outsideContext.y = 10.0f;
    EXPECT_EQ(face.touch(outsideContext), static_cast<EgLab::RE::IdxType>(-1));

    EgLab::RE::RenderLine nearerLine;
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> lineVertices;
    lineVertices.pushBack(EgLab::RE::CoordType(-0.5f, 0.0f, -0.5f));
    lineVertices.pushBack(EgLab::RE::CoordType(0.5f, 0.0f, -0.5f));
    nearerLine.setNodes(EgLab::Common::move(lineVertices));
    EgLab::Common::DynamicArray<EgLab::RE::IdxType> lineIndices;
    lineIndices.pushBack(0);
    lineIndices.pushBack(1);
    nearerLine.setIndices(EgLab::Common::move(lineIndices));

    EgLab::RE::PickContext depthContext = context;
    depthContext.bestDepth = std::numeric_limits<EgLab::RE::Scalar>::max();
    depthContext.bestDistanceSquared = std::numeric_limits<EgLab::RE::Scalar>::max();
    EXPECT_EQ(face.touch(depthContext), 0u);
    EXPECT_EQ(nearerLine.touch(depthContext), 0u);
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

    app.window->activeContext();
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

    app.window->deactiveContext();

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
    for (; meshIt.hasMoreNode(); meshIt.nextNode())
    {
        auto &n = meshIt.currentNode();
        EXPECT_EQ(n.getId(), i);
        i++;
    }

    auto &elem = meshIt.currentElem();
    EXPECT_EQ(elem.getId(), 1);

    EgLab::RE::MeshPrimitiveCreator creator(mesh);
    auto nodePrimitive = creator.getPrimitive<EgLab::RE::RenderNode>();
    auto linePrimitive = creator.getPrimitive<EgLab::RE::RenderLine>();
    auto facePrimitive = creator.getPrimitive<EgLab::RE::RenderFace>();
    app.window->activeContext();
    nodePrimitive->setup();
    linePrimitive->setup();
    facePrimitive->setup();
    app.window->deactiveContext();

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
    for (; meshIt.hasMoreNode(); meshIt.nextNode())
    {
        auto &n = meshIt.currentNode();
        EXPECT_EQ(n.getId(), i);
        i++;
    }

    auto &elem = meshIt.currentElem();

    EXPECT_EQ(elem.getId(), 1);

    EgLab::RE::MeshPrimitiveCreator creator(mesh);
    auto nodePrimitive = creator.getPrimitive<EgLab::RE::RenderNode>();
    auto linePrimitive = creator.getPrimitive<EgLab::RE::RenderLine>();
    auto facePrimitive = creator.getPrimitive<EgLab::RE::RenderFace>();

    app.window->activeContext();
    nodePrimitive->setup();
    linePrimitive->setup();
    facePrimitive->setup();
    app.window->deactiveContext();

    EgLab::Common::SharedPtr<EgLab::RE::Shader> shader;
    EgLab::RE::ShaderLib::instance().getNodeShader(shader);
    app.scene->addPrimitive(shader, nodePrimitive);

    EgLab::RE::ShaderLib::instance().getLineShader(shader);
    app.scene->addPrimitive(shader, linePrimitive);

    EgLab::RE::ShaderLib::instance().getFaceShader(shader);
    app.scene->addPrimitive(shader, facePrimitive);

    app.exec();
}

TEST_F(TestRender, importmesh)
{
    SimpleApp app;
    EgLab::ME::GmshImporter importer(EgLab::getTestDataDir("B27234.msh"));
    auto mesh = importer.getMesh();

    EgLab::RE::MeshPrimitiveCreator creator(mesh);
    auto nodePrimitive = creator.getPrimitive<EgLab::RE::RenderNode>();
    auto linePrimitive = creator.getPrimitive<EgLab::RE::RenderLine>();
    auto facePrimitive = creator.getPrimitive<EgLab::RE::RenderFace>();

    app.window->activeContext();
    nodePrimitive->setup();
    linePrimitive->setup();
    facePrimitive->setup();
    app.window->deactiveContext();

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
    EgLab::Common::SharedPtr<EgLab::RE::Shader> shader;
    EgLab::RE::ShaderLib::instance().getNodeShader(shader);
}