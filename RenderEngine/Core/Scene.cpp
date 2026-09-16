#include "RenderEngine/Core/Scene.hpp"

#include "RenderEngine/Core/CameraController.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/OrbitCameraController.hpp"

namespace EgLab::RE
{
    Scene::Scene()
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

    const RenderBuckets& Scene::getRenderBuckets() const
    {
        return _renderPrimitives;
    }

    const Common::BBox<Scalar, 3> &Scene::getBounds() const
    {
        return _sceneBounds;
    }
} // namespace EgLab::RE