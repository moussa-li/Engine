#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/UniquePtr.hpp"
#include "Definites.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngineAPI.hpp"

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

    class RenderEngineAPI Scene
    {
    public:
        explicit Scene();

        void update(DeltaTime);

        Return addEntity(SharedPtr<Entity> entity);

        EgLab::DynamicArray<SharedPtr<Entity>> getEntities() const;

    private:
        EgLab::DynamicArray<SharedPtr<Entity>> _entities;
    };
} // namespace EgLab