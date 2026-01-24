#include "Entity.hpp"

#include "Core/RenderMesh.hpp"
#include "Core/Shader.hpp"
#include "Core/Transform.hpp"

namespace EgLab
{

    void Entity::draw() const
    {
        for (const auto &m : _meshes)
        {
            Transform t;
            m->draw(_shader, t);
        }
    }

    void Entity::update(DeltaTime time)
    {
    }

} // namespace EgLab