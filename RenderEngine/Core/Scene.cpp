#include "RenderEngine/Core/Scene.hpp"

#include "RenderEngine/Core/CameraController.hpp"
#include "RenderEngine/Core/OrbitCameraController.hpp"

namespace EgLab
{
    Scene::Scene(InteractionModel mode)
    {
        switch (mode)
        {
        case InteractionModel::FPS:
            return;
        case InteractionModel::OBRIT:
            _controller = makeUnique<OrbitCameraController>();
            break;
        default:
            return;
        }

        _controller->setCamera(_camera);
    }

    void Scene::update(DeltaTime deltaTime)
    {
        if (_controller)
        {
            _controller->update(deltaTime);
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