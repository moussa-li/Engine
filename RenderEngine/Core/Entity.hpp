#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Core/Shader.hpp"
#include "Definites.hpp"


namespace EgLab
{
    class RenderMesh;
    class Shader;
    class Entity
    {
    public:
        virtual void draw() const;

        virtual void update(DeltaTime time);

    protected:
        DynamicArray<SharedPtr<RenderMesh>> _meshes;

        SharedPtr<Shader> _shader;
    };

} // namespace EgLab