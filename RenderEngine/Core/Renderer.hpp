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

        void update(DeltaTime deltaTime);

        Common::UniquePtr<RenderConfigure> &getConfigure();

        Renderer();

        ~Renderer() = default;

    private:
        void loadCamera(const Common::SharedPtr<Shader> &shader,
                        const Common::SharedPtr<Camera> &camera) const;

    private:
        Common::DynamicArray<RenderViewport> _viewports;

        Common::DynamicArray<Light *> _lights;

        Common::UniquePtr<RenderConfigure> _configure;
    };

} // namespace EgLab::RE