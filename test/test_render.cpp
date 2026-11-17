#include "test_render.h"

#include <GLFW/glfw3.h>

#include <chrono>

#include "Common/StringLineIterator.hpp"
#include "Common/String.hpp"
#include "MeshEngine/IO/GmshImporter.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/MeshPrimitiveCreator.hpp"
#include "RenderEngine/Core/MeshToTriFace.hpp"
#include "RenderEngine/Core/OrbitCameraController.hpp"
#include "RenderEngine/Core/PickManager.hpp"
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

        // 4. 鏄剧ず褰撳墠鍊硷紝鏂逛究璋冭瘯
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

        // 娓叉煋 UI
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

TEST_F(TestRender, set_camera_transitions_smoothly)
{
    auto camera = EgLab::Common::makeShared<EgLab::RE::Camera>(
        800, 600, EgLab::RE::CoordType(0.0f, 0.0f, 10.0f));
    EgLab::RE::OrbitCameraController controller;
    controller.setCamera(camera);

    const float startZ = camera->getPosition().z();
    const float endZ = 2.0f * std::sqrt(3.0f);
    camera->setCamera(EgLab::RE::CoordType(0.0f, 0.0f, endZ),
                      EgLab::RE::CoordType(0.0f, 0.0f, 0.0f), 2.0f);
    EXPECT_FLOAT_EQ(camera->getPosition().z(), startZ);

    controller.update(0.225f);
    const float middleZ = camera->getPosition().z();
    EXPECT_LT(middleZ, startZ);
    EXPECT_GT(middleZ, 3.0f);

    controller.update(0.225f);
    EXPECT_NEAR(camera->getPosition().z(), endZ, 0.001f);
}

TEST_F(TestRender, fit_view_uses_camera_transition)
{
    EgLab::RE::Camera camera(800, 600, EgLab::RE::CoordType(0.0f, 0.0f, 10.0f));
    EgLab::Common::BBox<EgLab::RE::Scalar, 3> bounds;
    bounds.addPoint(EgLab::RE::CoordType(-1.0f, -1.0f, -1.0f));
    bounds.addPoint(EgLab::RE::CoordType(1.0f, 1.0f, 1.0f));

    camera.fitView(bounds, 2.0f);
    EXPECT_FLOAT_EQ(camera.getPosition().z(), 10.0f);
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

TEST_F(TestRender, tet10HiddenElementsRebuildBoundaryTopology)
{
    auto mesh = EgLab::makeTwoTet10Mesh();

    EgLab::Common::SharedPtr<EgLab::RE::MeshToTriFace> translator =
        EgLab::Common::makeShared<EgLab::RE::MeshToTriFace>();
    translator->setMesh(mesh);

    // Six quadratic boundary faces, split into four triangles each.
    EXPECT_EQ(translator->getBoundaryFaceIdx().size(), 6u * 4u * 3u);
    EXPECT_EQ(translator->getBoundaryFaceLocations().size(), 6u * 4u);

    EgLab::Common::DynamicArray<EgLab::ME::IdType> hiddenElementIds;
    hiddenElementIds.pushBack(1u);
    translator->setHiddenElements(hiddenElementIds);

    // Hiding element 1 reveals the shared face, leaving all four faces of element 2.
    EXPECT_EQ(translator->getBoundaryFaceIdx().size(), 4u * 4u * 3u);
    const auto &locations = translator->getBoundaryFaceLocations();
    ASSERT_EQ(locations.size(), 4u * 4u);
    for (const auto &location : locations)
    {
        EXPECT_EQ(location.elemId, 2u);
    }
}
TEST_F(TestRender, camera_input_bindings_resolve_defaults)
{
    const EgLab::RE::CameraInputBindings bindings = EgLab::RE::CameraInputBindings::defaults();

    EXPECT_EQ(bindings.resolve(0, EgLab::RE::CameraModifier_None),
              EgLab::RE::CameraAction::Pan);
    EXPECT_EQ(bindings.resolve(1, EgLab::RE::CameraModifier_None),
              EgLab::RE::CameraAction::Rotate);
    EXPECT_EQ(bindings.resolve(1, EgLab::RE::CameraModifier_Ctrl),
              EgLab::RE::CameraAction::RotateLocal);
    EXPECT_EQ(bindings.resolve(2, EgLab::RE::CameraModifier_None),
              EgLab::RE::CameraAction::Zoom);
    EXPECT_EQ(bindings.resolve(2, EgLab::RE::CameraModifier_Shift),
              EgLab::RE::CameraAction::None);
}

TEST_F(TestRender, orbit_camera_local_rotation_uses_picked_mesh_center)
{
    auto camera = EgLab::Common::makeShared<EgLab::RE::Camera>(
        800, 600, EgLab::RE::CoordType(0.0f, 0.0f, 10.0f));
    EgLab::RE::OrbitCameraController controller;
    controller.setCamera(camera);
    controller.setInputBindings(EgLab::RE::CameraInputBindings::defaults());

    const EgLab::RE::CoordType pivot(3.0f, 2.0f, 1.0f);
    controller.setLocalPivotProvider([pivot](EgLab::RE::CoordType& center) {
        center = pivot;
        return true;
    });

    const float distanceBefore = (camera->getPosition() - pivot).length();
    const EgLab::RE::CoordType targetBefore = camera->getTarget();
    ASSERT_TRUE(controller.processMouseButton(1, 1, 400.0f, 300.0f,
                                             EgLab::RE::CameraModifier_Ctrl));
    ASSERT_TRUE(controller.processMouseMove(420.0f, 300.0f));

    EXPECT_NEAR((camera->getPosition() - pivot).length(), distanceBefore, 0.001f);
    EXPECT_NEAR((camera->getTarget() - targetBefore).length(), 0.0f, 0.001f);
}

TEST_F(TestRender, orbit_camera_global_rotation_keeps_target)
{
    auto camera = EgLab::Common::makeShared<EgLab::RE::Camera>(
        800, 600, EgLab::RE::CoordType(0.0f, 0.0f, 10.0f));
    EgLab::RE::OrbitCameraController controller;
    controller.setCamera(camera);
    controller.setInputBindings(EgLab::RE::CameraInputBindings::defaults());

    const EgLab::RE::CoordType targetBefore = camera->getTarget();
    ASSERT_TRUE(controller.processMouseButton(1, 1, 400.0f, 300.0f,
                                             EgLab::RE::CameraModifier_None));
    ASSERT_TRUE(controller.processMouseMove(420.0f, 300.0f));

    EXPECT_NEAR((camera->getTarget() - targetBefore).length(), 0.0f, 0.001f);
}

TEST_F(TestRender, orbit_camera_zoom_anchors_at_cursor)
{
    auto camera = EgLab::Common::makeShared<EgLab::RE::Camera>(
        800, 600, EgLab::RE::CoordType(0.0f, 0.0f, 10.0f));
    EgLab::RE::OrbitCameraController controller;
    controller.setCamera(camera);
    controller.setViewportSize(800.0f, 600.0f);
    controller.setZoomAnchor(EgLab::RE::CameraZoomAnchor::MouseCursor);

    const EgLab::RE::CoordType targetBefore = camera->getTarget();
    const float zoomBefore = camera->getZoom();
    ASSERT_TRUE(controller.processMouseScroll(1.0f, 800.0f, 300.0f));

    EXPECT_LT(camera->getZoom(), zoomBefore);
    EXPECT_GT(camera->getTarget().x(), targetBefore.x());
}

TEST_F(TestRender, orbit_camera_middle_drag_zooms)
{
    auto camera = EgLab::Common::makeShared<EgLab::RE::Camera>(
        800, 600, EgLab::RE::CoordType(0.0f, 0.0f, 10.0f));
    EgLab::RE::OrbitCameraController controller;
    controller.setCamera(camera);
    controller.setInputBindings(EgLab::RE::CameraInputBindings::defaults());
    controller.setViewportSize(800.0f, 600.0f);

    const float zoomBefore = camera->getZoom();
    ASSERT_TRUE(controller.processMouseButton(2, 1, 400.0f, 300.0f,
                                             EgLab::RE::CameraModifier_None));
    ASSERT_TRUE(controller.processMouseMove(400.0f, 250.0f));

    EXPECT_LT(camera->getZoom(), zoomBefore);
}

TEST_F(TestRender, pick_manager_tracks_acquired_kinds)
{
    EgLab::RE::PickManager manager;
    manager.releaseOwner(EgLab::Common::String("PickManagerTest"));
    EXPECT_EQ(manager.activeKinds(), 0u);

    const auto nodeToken =
        manager.acquire(EgLab::RE::PickKind::Node, EgLab::Common::String("PickManagerTest"));
    EXPECT_NE(nodeToken, EgLab::RE::PickManager::kInvalidToken);
    EXPECT_TRUE(manager.isActive(EgLab::RE::PickKind::Node));
    EXPECT_FALSE(manager.isActive(EgLab::RE::PickKind::Edge));
    EXPECT_EQ(manager.activeKinds(), EgLab::RE::toPickMask(EgLab::RE::PickKind::Node));

    const auto faceToken =
        manager.acquire(EgLab::RE::PickKind::Face, EgLab::Common::String("PickManagerTest"));
    EXPECT_EQ(manager.activeKinds(), EgLab::RE::toPickMask(EgLab::RE::PickKind::Node) |
                                         EgLab::RE::toPickMask(EgLab::RE::PickKind::Face));

    manager.release(nodeToken);
    EXPECT_FALSE(manager.isActive(EgLab::RE::PickKind::Node));
    EXPECT_TRUE(manager.isActive(EgLab::RE::PickKind::Face));

    manager.release(faceToken);
    EXPECT_EQ(manager.activeKinds(), 0u);
    EXPECT_EQ(manager.acquire(EgLab::RE::PickKind::None, EgLab::Common::String("PickManagerTest")),
              EgLab::RE::PickManager::kInvalidToken);
}

TEST_F(TestRender, pick_priority_prefers_node_then_edge_then_face)
{
    EgLab::RE::RenderNode node;
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> nodeVertices;
    nodeVertices.pushBack(EgLab::RE::CoordType(0.0f, 0.0f, 0.5f));
    node.setNodes(EgLab::Common::move(nodeVertices));

    EgLab::RE::RenderLine line;
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> lineVertices;
    lineVertices.pushBack(EgLab::RE::CoordType(-0.5f, 0.0f, 0.5f));
    lineVertices.pushBack(EgLab::RE::CoordType(0.5f, 0.0f, 0.5f));
    line.setNodes(EgLab::Common::move(lineVertices));
    EgLab::Common::DynamicArray<EgLab::RE::IdxType> lineIndices;
    lineIndices.pushBack(0);
    lineIndices.pushBack(1);
    line.setIndices(EgLab::Common::move(lineIndices));

    EgLab::RE::RenderFace face;
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> faceVertices;
    faceVertices.pushBack(EgLab::RE::CoordType(-0.5f, -0.5f, 0.5f));
    faceVertices.pushBack(EgLab::RE::CoordType(0.5f, -0.5f, 0.5f));
    faceVertices.pushBack(EgLab::RE::CoordType(0.0f, 0.5f, 0.5f));
    face.setNodes(EgLab::Common::move(faceVertices));
    EgLab::Common::DynamicArray<EgLab::RE::IdxType> faceIndices;
    faceIndices.pushBack(0);
    faceIndices.pushBack(1);
    faceIndices.pushBack(2);
    face.setIndices(EgLab::Common::move(faceIndices));

    EXPECT_EQ(node.getPickKind(), EgLab::RE::PickKind::Node);
    EXPECT_EQ(line.getPickKind(), EgLab::RE::PickKind::Edge);
    EXPECT_EQ(face.getPickKind(), EgLab::RE::PickKind::Face);

    const auto makeContext = []() {
        EgLab::RE::PickContext context;
        context.width = 200;
        context.height = 200;
        context.x = 100.0f;
        context.y = 100.0f;
        context.view = EgLab::Common::Matrix4f::Identity();
        context.projection = EgLab::Common::Matrix4f::Identity();
        return context;
    };

    // Face first, node second: the node wins on a coincident hit.
    EgLab::RE::PickContext faceThenNode = makeContext();
    EXPECT_EQ(face.touch(faceThenNode), 0u);
    EXPECT_EQ(node.touch(faceThenNode), 0u);

    // Face first, edge second: the edge wins.
    EgLab::RE::PickContext faceThenEdge = makeContext();
    EXPECT_EQ(face.touch(faceThenEdge), 0u);
    EXPECT_EQ(line.touch(faceThenEdge), 0u);

    // Edge first, node second: the node wins.
    EgLab::RE::PickContext edgeThenNode = makeContext();
    EXPECT_EQ(line.touch(edgeThenNode), 0u);
    EXPECT_EQ(node.touch(edgeThenNode), 0u);

    // A genuinely farther node still loses to a closer face.
    EgLab::RE::RenderNode farNode;
    EgLab::Common::DynamicArray<EgLab::RE::CoordType> farNodeVertices;
    farNodeVertices.pushBack(EgLab::RE::CoordType(0.0f, 0.0f, 0.6f));
    farNode.setNodes(EgLab::Common::move(farNodeVertices));

    EgLab::RE::PickContext depthWins = makeContext();
    EXPECT_EQ(face.touch(depthWins), 0u);
    EXPECT_EQ(farNode.touch(depthWins), static_cast<EgLab::RE::IdxType>(-1));
}
