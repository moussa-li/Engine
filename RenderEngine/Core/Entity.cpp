#include "Entity.hpp"

#include "Core/Camera.hpp"
#include "Core/RenderMesh.hpp"
#include "Core/Shader.hpp"
#include "Core/ShaderLib.hpp"
#include "Core/Transform.hpp"

namespace EgLab
{
    Entity::Entity(Vector3f position, Vector3f rotation, Vector3f scale)
        : _transform(position, rotation, scale)
    {
        _shader = makeShared<Shader>(ShaderId::Basic);
        _shader->bind();

        _shader->setUniform1i("material.texture_specular1", 1);
        _shader->setUniform1i("material.texture_diffuse1", 1);
        _shader->setUniform1f("material.shininess", 32.0f);
    }

    void Entity::draw() const
    {
        for (const auto &m : _meshes)
        {
            m->draw(_shader, _transform);
        }
    }

    void Entity::update(DeltaTime time)
    {
    }

    void Entity::loadCamera(const SharedPtr<Camera> &camera)
    {
        Matrix4f proj = Matrix4f::Identity();
        proj = camera->perspective();
        Matrix4f view = camera->view();
        _shader->bind();

        _shader->setUniformMat4f("proj", proj);
        _shader->setUniformMat4f("view", view);
        //_shader->setUniform3f("viewPos", camera->getPosition());
        //_shader->setUniformMat4f("mvp", proj);

        _shader->unBind();
    }

} // namespace EgLab