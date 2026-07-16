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
    EgLab::SharedPtr<EgLab::Window> w;
    EgLab::SharedPtr<EgLab::Renderer> r;

    EgLab::DeltaTime lastFrame;
    EgLab::DeltaTime deltaTime;

    SimpleApp() : w(EgLab::makeShared<EgLab::Window>(300, 300))
    {
        auto c = EgLab::makeShared<EgLab::Camera>(300, 300);
        r = EgLab::makeShared<EgLab::Renderer>();
        EgLab::RenderViewport viewport;
        viewport.camera = c;
        viewport.window = w;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = 300;
        viewport.height = 300;
        r->addViewport(viewport);

        EgLab::Vector3f pos(0, 0, 0);
        EgLab::Vector3f rot(0, 0, 0);
        EgLab::Vector3f scal(10, 10, 10);
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

            w->deal();
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