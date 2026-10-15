#pragma once
#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Definites.hpp"
#include "RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class Window;
    class Camera;
    class Entity;
    class Light;
    class Scene;
    class Shader;
    class RenderConfigure;
    class RenderPrimitive;
    class Selector;

    struct RenderViewport
    {
        Common::SharedPtr<Window> window;
        int x, y;          // start position
        int width, height; // size
    };

    class RenderEngineAPI Renderer
    {
    public:
        void addViewport(RenderViewport &);

        void clear();

        void draw(const Common::SharedPtr<Scene> &scene,
                  const Common::SharedPtr<Camera> &camera) const;

        bool tryGetPickedNode(Common::SharedPtr<RenderPrimitive> &primitive,
                              IdxType &nodeIndex) const;

        void setBackGroundColor(const Common::Vector4f &color);

        Common::Vector4f getBackGroundColor() const;

        void update(DeltaTime deltaTime);

        Common::UniquePtr<RenderConfigure> &getConfigure();

        void setSelector(Common::SharedPtr<Selector> selector);
        Common::SharedPtr<Selector> getSelector() const;

        void setRenderNodesEnabled(bool enabled);
        bool isRenderNodesEnabled() const;
        void setRenderEdgesEnabled(bool enabled);
        bool isRenderEdgesEnabled() const;
        void setRenderFacesEnabled(bool enabled);
        bool isRenderFacesEnabled() const;

        Renderer();

        ~Renderer();

    private:
        void loadCamera(const Common::SharedPtr<Shader> &shader,
                        const Common::SharedPtr<Camera> &camera,
                        const Common::SharedPtr<Scene> &scene) const;
        bool isPrimitiveVisible(const Common::SharedPtr<RenderPrimitive> &primitive) const;

    private:
        Common::DynamicArray<RenderViewport> _viewports;

        Common::DynamicArray<Light *> _lights;

        Common::UniquePtr<RenderConfigure> _configure;
        Common::SharedPtr<Selector> _selector;

        Common::Vector4f _backGroundColor;

        mutable Common::SharedPtr<RenderPrimitive> _pickedPrimitive{nullptr};
        mutable IdxType _pickedNodeIndex{static_cast<IdxType>(-1)};
        bool _renderNodesEnabled{true};
        bool _renderEdgesEnabled{true};
        bool _renderFacesEnabled{true};
    };

} // namespace EgLab::RE