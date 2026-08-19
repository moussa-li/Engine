#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Core/RenderEngineAPI.hpp"
#include "Core/Shader.hpp"
#include "Core/Transform.hpp"
#include "Definites.hpp"

namespace EgLab::RE
{
    class RenderMesh;
    class RenderNode;
    class Shader;
    class Camera;
    class RenderPrimitive;

    /**
     * @brief the basic of any render object
     *
     */
    class RenderEngineAPI Entity
    {
    public:
        Entity();

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
        void loadCamera(const Common::SharedPtr<Camera> &camera);

    protected:
        Common::DynamicArray<Common::SharedPtr<RenderPrimitive>> _renderPrimitives;

        Common::DynamicArray<Common::SharedPtr<Shader>> _shaders;
    };

} // namespace EgLab::RE