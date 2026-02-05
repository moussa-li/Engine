#include "test_render.h"

#include "Common/StringLineIterator.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/RenderConfigure.hpp"
#include "RenderEngine/Core/Renderer.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"
#include "RenderEngine/Core/Window.hpp"
#include "RenderEngine/Objects/Box.hpp"

class SimpleApp
{
public:
    EgLab::UniquePtr<EgLab::Window> w;
    EgLab::UniquePtr<EgLab::Renderer> r;

    EgLab::DeltaTime lastFrame;
    EgLab::DeltaTime deltaTime;

    SimpleApp() : w(EgLab::makeUnique<EgLab::Window>(300, 300))
    {
        auto c = EgLab::makeUnique<EgLab::Camera>();
        r = EgLab::makeUnique<EgLab::Renderer>(move(c));

        EgLab::Vector3f pos(0, 0, 0);
        EgLab::Vector3f rot(0, 0, 0);
        EgLab::Vector3f scal(1, 1, 1);
        auto box = EgLab::makeShared<EgLab::Box>(pos, rot, scal);

        r->addEntity(box);
    }

    ~SimpleApp() = default;
    void exec()
    {
        while (w->shouldClose() == false)
        {
            float currentFrame = w->getTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            r->update(deltaTime);

            r->draw();

            /*input*/
            // CAMERA->processInput(window, deltaTime);

            /* Swap front and back buffers */
            // glfwSwapBuffers(_window);

            /* Poll for and process events */
            // glfwPollEvents();

            // CalculateFrameRate();
        };
    }
};

TEST_F(TestRender, window)
{
    SimpleApp app;
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