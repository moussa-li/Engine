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
    class Camera;

    /**
     * @brief the basic of any render object
     *
     */
    class RenderEngineAPI Entity
    {
    public:
        Entity(Vector3f position, Vector3f rotation, Vector3f scale);

        /**
         * @brief draw the meshes in the entity
         *
         */
        virtual void draw() const;

        /**
         * @brief
         *
         * @param time
         */
        virtual void update(DeltaTime time);

        /**
         * @brief which camera show this entity
         *
         * @param camera
         */
        void loadCamera(const SharedPtr<Camera> &camera);

    protected:
        DynamicArray<SharedPtr<RenderMesh>> _meshes;

        SharedPtr<Shader> _shader;

        Transform _transform;
    };

} // namespace EgLab