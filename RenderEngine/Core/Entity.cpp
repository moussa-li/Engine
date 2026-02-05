#include "Entity.hpp"

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

} // namespace EgLab