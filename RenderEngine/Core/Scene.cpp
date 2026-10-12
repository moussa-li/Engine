#include "RenderEngine/Core/Scene.hpp"

#include "RenderEngine/Core/CameraController.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/OrbitCameraController.hpp"

namespace EgLab::RE
{
    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    void Scene::update(DeltaTime deltaTime)
    {
    }

    Common::Return Scene::addPrimitive(Common::SharedPtr<Shader> shader,
                                       Common::SharedPtr<RenderPrimitive> primitive)
    {
        if (shader == nullptr || primitive == nullptr) return Common::Return::Failed;

        _renderPrimitives[shader].pushBack(primitive);
        _sceneBounds.merge(primitive->getBounds());
        return Common::Return::Succeed;
    }

    Common::Return Scene::removePrimitive(const Common::SharedPtr<Shader>& shader,
                                          const Common::SharedPtr<RenderPrimitive>& primitive)
    {
        auto& bucket = _renderPrimitives[shader];
        Common::DynamicArray<Common::SharedPtr<RenderPrimitive>> remaining;
        bool removed = false;
        for (auto& item : bucket)
        {
            if (item == primitive)
            {
                removed = true;
            }
            else
            {
                remaining.pushBack(item);
            }
        }

        if (!removed)
        {
            return Common::Return::Failed;
        }

        bucket = Common::move(remaining);

        _sceneBounds.reset();
        for (auto it = _renderPrimitives.begin(); it.hasNext(); ++it)
        {
            for (const auto& remainingPrimitive : it.data().second)
            {
                _sceneBounds.merge(remainingPrimitive->getBounds());
            }
        }

        return Common::Return::Succeed;
    }

    const RenderBuckets& Scene::getRenderBuckets() const
    {
        return _renderPrimitives;
    }

    const Common::BBox<Scalar, 3>& Scene::getBounds() const
    {
        return _sceneBounds;
    }
} // namespace EgLab::RE