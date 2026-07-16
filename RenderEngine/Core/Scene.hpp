#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/UniquePtr.hpp"
#include "Definites.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Definites.hpp"

namespace EgLab
{
    class Camera;
    class CameraController;
    class Entity;

    enum class InteractionModel
    {
        FPS,
        OBRIT,
        UNKNOWN
    };

    class Scene
    {
    public:
        explicit Scene(InteractionModel mode);

        void update(DeltaTime);

        Return addEntity(SharedPtr<Entity> entity);

        EgLab::DynamicArray<SharedPtr<Entity>> getEntities() const;

    private:
        SharedPtr<Camera> _camera;
        UniquePtr<CameraController> _controller;

        EgLab::DynamicArray<SharedPtr<Entity>> _entities;
    };
} // namespace EgLab