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
    class RenderConfigure;

    class RenderEngineAPI Renderer
    {
    public:
        void setWindow(SharedPtr<Window>);

        void clear();

        void draw() const;

        /*
        void insertEntity(Entity* entity);

        void insertLight(Light* light);
        */

        Return addEntity(SharedPtr<Entity> entity);

        void update(DeltaTime deltaTime);

        UniquePtr<RenderConfigure> &getConfigure();

        Renderer(UniquePtr<Camera> &&camera);

        ~Renderer() = default;

    private:
        EgLab::DynamicArray<SharedPtr<Entity>> _entities;

        EgLab::DynamicArray<Light *> _lights;

        UniquePtr<Camera> _camera;

        UniquePtr<RenderConfigure> _configure;
    };

} // namespace EgLab