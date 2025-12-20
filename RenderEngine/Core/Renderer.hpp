#pragma once
#include "Common/DynamicArray.hpp"
#include "Definites.hpp"
#include "RenderEngineAPI.hpp"


namespace EgLab
{
    class Window;
    class Camera;
    class Entity;
    class Light;

    class RenderEngineAPI Renderer
    {
    public:
        void setWindow(Window*);

        void clear();

        void draw() const;

        /*
        void insertEntity(Entity* entity);

        void insertLight(Light* light);
        */

        void update(DeltaTime deltaTime);

        Renderer(Camera* camera);

    private:
        EgLab::DynamicArray<Entity*> _entities;

        EgLab::DynamicArray<Light*> _lights;

        Camera* _camera;
    };

} // namespace EgLab