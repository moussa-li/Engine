#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Core/RenderEngineAPI.hpp"
#include "Core/Shader.hpp"
#include "Core/Transform.hpp"
#include "Definites.hpp"

namespace EgLab
{
    class RenderMesh;
    class Shader;
    class RenderEngineAPI Entity
    {
    public:
        Entity(Vector3f position, Vector3f rotation, Vector3f scale);

        virtual void draw() const;

        virtual void update(DeltaTime time);

    protected:
        DynamicArray<SharedPtr<RenderMesh>> _meshes;

        SharedPtr<Shader> _shader;

        Transform _transform;
    };

} // namespace EgLab