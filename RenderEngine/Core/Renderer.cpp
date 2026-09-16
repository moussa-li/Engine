#include "Renderer.hpp"

#include <GL/glew.h>

#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/RenderConfigure.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/Scene.hpp"
#include "RenderEngine/Core/Window.hpp"

namespace EgLab::RE
{
    void Renderer::clear()
    {
    }

    void Renderer::addViewport(RenderViewport &viewport)
    {
        _viewports.pushBack(viewport);
    }

    void Renderer::update(DeltaTime time)
    {
        if (_configure->enableDeepTest.get())
        {
            glEnable(GL_DEPTH_TEST);
            switch (_configure->deepTestType.get())
            {
            case RenderConfigure::DeepTestType::LESS:
                glDepthFunc(GL_LESS);
                break;
                // TODO:add other deep test type
            default:
                glDepthFunc(GL_LESS);
                break;
            }
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::loadCamera(const Common::SharedPtr<Shader> &shader,
                              const Common::SharedPtr<Camera> &camera,
                              const Common::SharedPtr<Scene> &scene) const
    {
        Common::Matrix4f proj = Common::Matrix4f::Identity();
        proj = camera->perspective(scene->getBounds());
        Common::Matrix4f view = camera->view();
        shader->bind();

        shader->setUniformMat4f("proj", proj);
        shader->setUniformMat4f("view", view);

        shader->unBind();
    }

    void Renderer::draw(const Common::SharedPtr<Scene> &scene,
                        const Common::SharedPtr<Camera> &camera) const
    {
        for (IdxType i = 0; i < _viewports.size(); ++i)
        {
            const auto &viewport = _viewports[i];
            const auto &renderBuckets = scene->getRenderBuckets();

            if (viewport.window == nullptr)
            {
                continue;
            }

            unsigned int framebufferWidth = 0;
            unsigned int framebufferHeight = 0;
            viewport.window->getFramebufferSize(framebufferWidth, framebufferHeight);
            if (framebufferWidth == 0 || framebufferHeight == 0)
            {
                continue;
            }

            glViewport(viewport.x, viewport.y,
                       static_cast<GLsizei>(framebufferWidth),
                       static_cast<GLsizei>(framebufferHeight));
            camera->setWH(framebufferWidth, framebufferHeight);

            for (auto it = renderBuckets.begin(); it.hasNext(); it.next())
            {
                const auto &shader = it.data().first;
                const auto &primitives = it.data().second;

                loadCamera(shader, camera, scene);

                for (auto primitiveIt = primitives.begin(); primitiveIt.hasNext();
                     primitiveIt.next())
                {
                    primitiveIt.data()->draw(shader);
                }
            }
        }
    }

    Common::UniquePtr<RenderConfigure> &Renderer::getConfigure()
    {
        return _configure;
    }

    Renderer::Renderer() : _configure(Common::makeUnique<RenderConfigure>())
    {
    }

    Renderer::~Renderer()
    {
    }

} // namespace EgLab::RE