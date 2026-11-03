#include "RenderEngine/Core/Entity.hpp"

#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/RenderMesh.hpp"
#include "RenderEngine/Core/Shader.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"
#include "RenderEngine/Core/Transform.hpp"

namespace EgLab::RE
{
    Entity::Entity()
    {
    }

    void Entity::update(DeltaTime time)
    {
    }

    void Entity::loadCamera(const Common::SharedPtr<Camera> &camera)
    {
        /* Matrix4f proj = Matrix4f::Identity();
        proj = camera->perspective();
        Matrix4f view = camera->view();
        _shader->bind();

        _shader->setUniformMat4f("proj", proj);
        _shader->setUniformMat4f("view", view);

        _shader->unBind(); */
    }

} // namespace EgLab::RE