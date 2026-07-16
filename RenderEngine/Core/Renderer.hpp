#pragma once
#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Definites.hpp"
#include "RenderEngineAPI.hpp"

namespace EgLab
{
    class Window;
    class Camera;
    class Entity;
    class Light;
    class Scene;
    class RenderConfigure;

    struct RenderViewport
    {
        SharedPtr<Camera> camera;
        SharedPtr<Window> window;
        int x, y;          // start position
        int width, height; // size
    };

    class RenderEngineAPI Renderer
    {
    public:
        void addViewport(RenderViewport &);

        void clear();

        void draw(const Scene &scene, const Camera &camera) const;

        Return addEntity(SharedPtr<Entity> entity);

        void update(DeltaTime deltaTime);

        UniquePtr<RenderConfigure> &getConfigure();

        Renderer();

        ~Renderer() = default;

    private:
        EgLab::DynamicArray<SharedPtr<Entity>> _entities;

        EgLab::DynamicArray<RenderViewport> _viewports;

        EgLab::DynamicArray<Light *> _lights;

        UniquePtr<RenderConfigure> _configure;
    };

} // namespace EgLab