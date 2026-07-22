#include "RenderEngine/Core/Scene.hpp"

#include "RenderEngine/Core/CameraController.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/OrbitCameraController.hpp"

namespace EgLab
{
    Scene::Scene()
    {
    }

    void Scene::update(DeltaTime deltaTime)
    {
        for (size_t i = 0; i < _entities.size(); ++i)
        {
            _entities[i]->update(deltaTime);
        }
    }

    EgLab::DynamicArray<SharedPtr<Entity>> Scene::getEntities() const
    {
        return _entities;
    }

    Return Scene::addEntity(SharedPtr<Entity> entity)
    {
        if (entity == nullptr) return Return::Failed;
        _entities.pushBack(entity);

        return Return::Succeed;
    }
} // namespace EgLab