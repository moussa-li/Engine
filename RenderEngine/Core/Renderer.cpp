#include "Renderer.hpp"

#include <GL/glew.h>

#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/MeshSelector.hpp"
#include "RenderEngine/Core/RenderConfigure.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/Scene.hpp"
#include "RenderEngine/Core/Selector.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"
#include "RenderEngine/Core/Window.hpp"

namespace EgLab::RE
{
    namespace
    {
        constexpr const char *RENDER_SETTINGS_FILE = "render_settings.ini";

        Common::Vector4f loadBackGroundColor()
        {
            Common::Vector4f color(1, 1, 1, 1);
            std::ifstream settings(RENDER_SETTINGS_FILE);
            std::string line;
            bool inRendererSection = false;
            while (std::getline(settings, line))
            {
                if (line == "[Renderer]")
                {
                    inRendererSection = true;
                    continue;
                }
                if (!line.empty() && line.front() == '[')
                {
                    inRendererSection = false;
                    continue;
                }
                constexpr const char *key = "BackgroundColor=";
                if (!inRendererSection || line.rfind(key, 0) != 0)
                {
                    continue;
                }

                Scalar channels[4]{};
                char separator1 = 0;
                char separator2 = 0;
                char separator3 = 0;
                std::istringstream value(line.substr(std::char_traits<char>::length(key)));
                if (value >> channels[0] >> separator1 >> channels[1] >> separator2 >>
                        channels[2] >> separator3 >> channels[3] &&
                    separator1 == ',' && separator2 == ',' && separator3 == ',' &&
                    std::isfinite(channels[0]) && std::isfinite(channels[1]) &&
                    std::isfinite(channels[2]) && std::isfinite(channels[3]) && channels[0] >= 0 &&
                    channels[0] <= 1 && channels[1] >= 0 && channels[1] <= 1 && channels[2] >= 0 &&
                    channels[2] <= 1 && channels[3] >= 0 && channels[3] <= 1)
                {
                    return Common::Vector4f(channels[0], channels[1], channels[2], channels[3]);
                }
            }
            return color;
        }

        void saveBackGroundColor(const Common::Vector4f &color)
        {
            std::ofstream settings(RENDER_SETTINGS_FILE, std::ios::trunc);
            if (!settings)
            {
                return;
            }
            settings << "[Renderer]\nBackgroundColor="
                     << std::setprecision(std::numeric_limits<Scalar>::max_digits10) << color[0]
                     << ',' << color[1] << ',' << color[2] << ',' << color[3] << '\n';
        }
    } // namespace

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
        shader->setUniform3f("viewPos", camera->getPosition());

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
                if (_selector)
                {
                    _selector->processPick(nullptr, static_cast<IdxType>(-1));
                }
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

            Common::SharedPtr<Shader> highlightShader;
            ShaderLib::instance().getHighlightShader(highlightShader);

            for (auto it = renderBuckets.begin(); it.hasNext(); it.next())
            {
                const auto &shader = it.data().first;
                const auto &primitives = it.data().second;

                const bool deferredHighlight = shader == highlightShader;
                if (!deferredHighlight)
                {
                    loadCamera(shader, camera, scene);
                    loadLight(shader, camera);
                }

                for (auto primitiveIt = primitives.begin(); primitiveIt.hasNext();
                     primitiveIt.next())
                {
                    if (!isPrimitiveVisible(primitiveIt.data()))
                    {
                        continue;
                    }
                    if (!deferredHighlight)
                    {
                        primitiveIt.data()->draw(shader);
                        if (selected)
                        {
                            const IdxType nodeIndex = primitiveIt.data()->touch(pickContext);
                            if (nodeIndex != static_cast<IdxType>(-1))
                            {
                                _pickedPrimitive = primitiveIt.data();
                                _pickedNodeIndex = nodeIndex;
                                if (_selector)
                                {
                                    _selector->processPick(_pickedPrimitive, _pickedNodeIndex);
                                }
                            }
                        }
                    }
                }
            }

            if (highlightShader != nullptr)
            {
                for (auto it = renderBuckets.begin(); it.hasNext(); it.next())
                {
                    const auto &shader = it.data().first;
                    if (shader != highlightShader)
                    {
                        continue;
                    }
                    loadCamera(shader, camera, scene);
                    loadLight(shader, camera);
                    for (auto primitiveIt = it.data().second.begin(); primitiveIt.hasNext();
                         primitiveIt.next())
                    {
                        if (isPrimitiveVisible(primitiveIt.data()))
                        {
                            primitiveIt.data()->draw(shader);
                        }
                    }
                    break;
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
                        if (primitiveIt.data() == _pickedPrimitive &&
                            isPrimitiveVisible(primitiveIt.data()))
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

    Common::SharedPtr<Selector> Renderer::getSelector() const
    {
        return _selector;
    }

    void Renderer::setRenderNodesEnabled(bool enabled)
    {
        _renderNodesEnabled = enabled;
    }

    bool Renderer::isRenderNodesEnabled() const
    {
        return _renderNodesEnabled;
    }

    void Renderer::setRenderEdgesEnabled(bool enabled)
    {
        _renderEdgesEnabled = enabled;
    }

    bool Renderer::isRenderEdgesEnabled() const
    {
        return _renderEdgesEnabled;
    }

    void Renderer::setRenderFacesEnabled(bool enabled)
    {
        _renderFacesEnabled = enabled;
    }

    bool Renderer::isRenderFacesEnabled() const
    {
        return _renderFacesEnabled;
    }

    bool Renderer::isPrimitiveVisible(const Common::SharedPtr<RenderPrimitive> &primitive) const
    {
        if (!primitive)
        {
            return false;
        }
        if (dynamic_cast<const RenderNode *>(primitive.get()) != nullptr)
        {
            return _renderNodesEnabled;
        }
        if (dynamic_cast<const RenderLine *>(primitive.get()) != nullptr)
        {
            return _renderEdgesEnabled;
        }
        if (dynamic_cast<const RenderFace *>(primitive.get()) != nullptr)
        {
            return _renderFacesEnabled;
        }
        return true;
    }

    void Renderer::setBackGroundColor(const Common::Vector4f &color)
    {
        bool changed = false;
        for (size_t channel = 0; channel < 4; ++channel)
        {
            changed = changed || _backGroundColor[channel] != color[channel];
        }
        if (!changed)
        {
            return;
        }

        _backGroundColor = color;
        saveBackGroundColor(_backGroundColor);
    }

    Common::Vector4f Renderer::getBackGroundColor() const
    {
        return _backGroundColor;
    }

    void Renderer::setLightSettings(const DirectionalLightSettings &settings)
    {
        std::lock_guard<std::mutex> lock(_lightMutex);
        _lightSettings = settings;
    }

    DirectionalLightSettings Renderer::getLightSettings() const
    {
        std::lock_guard<std::mutex> lock(_lightMutex);
        return _lightSettings;
    }

    void Renderer::loadLight(const Common::SharedPtr<Shader> &shader,
                             const Common::SharedPtr<Camera> &camera) const
    {
        const DirectionalLightSettings settings = getLightSettings();
        shader->bind();
        shader->setUniform3f("pointLights.position", camera->getPosition());
        shader->setUniform1f("pointLights.constant", 1.0f);
        shader->setUniform1f("pointLights.linear", 0.0f);
        shader->setUniform1f("pointLights.quadratic", 0.0f);
        shader->setUniform3f("pointLights.ambient", settings.ambient);
        shader->setUniform3f("pointLights.diffuse", settings.diffuse);
        shader->setUniform3f("pointLights.specular", settings.specular);
        shader->setUniform3f("dirLight.direction", Common::Vector3f(-0.2f, -1.0f, -0.3f));
        shader->setUniform3f("dirLight.ambient", Common::Vector3f(0.15f, 0.15f, 0.15f));
        shader->setUniform3f("dirLight.diffuse", Common::Vector3f(0.4f, 0.4f, 0.4f));
        shader->setUniform3f("dirLight.specular", Common::Vector3f(0.35f, 0.35f, 0.35f));
        shader->setUniform1f("material.shininess", settings.shininess);
        shader->unBind();
    }

    Renderer::Renderer() : _configure(Common::makeUnique<RenderConfigure>())
    {
        auto meshSelector = Common::makeShared<MeshSelector>();
        _selector = meshSelector;
        _backGroundColor = loadBackGroundColor();
    }

    Renderer::~Renderer()
    {
    }

} // namespace EgLab::RE