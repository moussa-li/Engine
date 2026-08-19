#include "Entity.hpp"

#include "Core/Camera.hpp"
#include "Core/RenderMesh.hpp"
#include "Core/Shader.hpp"
#include "Core/ShaderLib.hpp"
#include "Core/Transform.hpp"

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