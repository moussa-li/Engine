#include "test_render.h"

#include "Common/StringLineIterator.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/RenderConfigure.hpp"
#include "RenderEngine/Core/Renderer.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"
#include "RenderEngine/Core/Window.hpp"

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
    class A
    {
    public:
        int* tt;
        int bb;
    };
    A a;
    a.tt = new int(32);
    a.tt[0] = 2;
    a.tt[1] = 5;
    a.tt[2] = 3;
    a.bb = 4;
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