#include "Renderer.hpp"

#include <GL/glew.h>

#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/MeshSelector.hpp"
#include "RenderEngine/Core/RenderConfigure.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/Scene.hpp"
#include "RenderEngine/Core/Selector.hpp"
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
        glEnable(GL_DEPTH_CLAMP);

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

        glClearColor(_backGroundColor[0], _backGroundColor[1], _backGroundColor[2],
                     _backGroundColor[3]);
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

            glViewport(viewport.x, viewport.y, static_cast<GLsizei>(framebufferWidth),
                       static_cast<GLsizei>(framebufferHeight));
            camera->setWH(framebufferWidth, framebufferHeight);

            Scalar cursorX = 0.0f;
            Scalar cursorY = 0.0f;
            const bool selected = viewport.window->trySelectedPos(cursorX, cursorY);
            if (selected)
            {
                _pickedPrimitive = nullptr;
                _pickedNodeIndex = static_cast<IdxType>(-1);
            }

            unsigned int windowWidth = 0;
            unsigned int windowHeight = 0;
            viewport.window->getWindowSize(windowWidth, windowHeight);
            if (windowWidth == 0 || windowHeight == 0)
            {
                continue;
            }

            PickContext pickContext;
            pickContext.x = cursorX * static_cast<Scalar>(framebufferWidth) / windowWidth -
                            static_cast<Scalar>(viewport.x);
            pickContext.y = cursorY * static_cast<Scalar>(framebufferHeight) / windowHeight +
                            static_cast<Scalar>(viewport.y);
            pickContext.width = framebufferWidth;
            pickContext.height = framebufferHeight;
            pickContext.view = camera->view();
            pickContext.projection = camera->perspective(scene->getBounds());

            for (auto it = renderBuckets.begin(); it.hasNext(); it.next())
            {
                const auto &shader = it.data().first;
                const auto &primitives = it.data().second;

                loadCamera(shader, camera, scene);

                for (auto primitiveIt = primitives.begin(); primitiveIt.hasNext();
                     primitiveIt.next())
                {
                    primitiveIt.data()->draw(shader);
                    if (selected)
                    {
                        const IdxType nodeIndex = primitiveIt.data()->touch(pickContext);
                        if (nodeIndex != static_cast<IdxType>(-1))
                        {
                            LOG(INFO) << "Picked node index: " << nodeIndex;
                            _pickedPrimitive = primitiveIt.data();
                            _pickedNodeIndex = nodeIndex;
                        }
                    }
                }
            }

            if (_pickedPrimitive != nullptr && _pickedNodeIndex != static_cast<IdxType>(-1))
            {
                Common::DynamicArray<IdxType> highlightIndices;
                if (_selector != nullptr)
                {
                    highlightIndices =
                        _selector->getHighlightIndices(_pickedPrimitive, _pickedNodeIndex);
                }
                else
                {
                    highlightIndices.pushBack(_pickedNodeIndex);
                }

                for (auto it = renderBuckets.begin(); it.hasNext(); it.next())
                {
                    const auto &shader = it.data().first;
                    const auto &primitives = it.data().second;
                    for (auto primitiveIt = primitives.begin(); primitiveIt.hasNext();
                         primitiveIt.next())
                    {
                        if (primitiveIt.data() == _pickedPrimitive)
                        {
                            for (IdxType index = 0; index < highlightIndices.size(); ++index)
                            {
                                primitiveIt.data()->drawHighlight(shader, highlightIndices[index]);
                            }
                        }
                    }
                }
            }
        }
    }

    bool Renderer::tryGetPickedNode(Common::SharedPtr<RenderPrimitive> &primitive,
                                    IdxType &nodeIndex) const
    {
        if (_pickedPrimitive == nullptr || _pickedNodeIndex == static_cast<IdxType>(-1))
        {
            return false;
        }

        primitive = _pickedPrimitive;
        nodeIndex = _pickedNodeIndex;
        return true;
    }

    Common::UniquePtr<RenderConfigure> &Renderer::getConfigure()
    {
        return _configure;
    }

    void Renderer::setSelector(Common::SharedPtr<Selector> selector)
    {
        _selector = Common::move(selector);
    }

    void Renderer::setBackGroundColor(const Common::Vector4f &color)
    {
        _backGroundColor = color;
    }

    Common::Vector4f Renderer::getBackGroundColor() const
    {
        return _backGroundColor;
    }

    Renderer::Renderer() : _configure(Common::makeUnique<RenderConfigure>())
    {
        auto meshSelector = Common::makeShared<MeshSelector>();
        _selector = meshSelector;
        _backGroundColor = Common::Vector4f(1, 1, 1, 1);
    }

    Renderer::~Renderer()
    {
    }

} // namespace EgLab::RE