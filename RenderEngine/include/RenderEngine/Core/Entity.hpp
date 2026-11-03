#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/Shader.hpp"
#include "RenderEngine/Core/Transform.hpp"
#include "RenderEngine/Core/Definites.hpp"

namespace EgLab::RE
{
    class RenderMesh;
    class RenderNode;
    class Shader;
    class Camera;
    class RenderPrimitive;

    /**
     * @brief Base class for objects that contribute primitives to a scene.
     */
    class RenderEngineAPI Entity
    {
    public:
        Entity();

        /**
         * @brief Updates the entity and its time-dependent state.
         * @param time Elapsed time since the previous update.
         */
        virtual void update(DeltaTime time);

        /**
         * @brief Supplies the camera used to render this entity.
         * @param camera Camera whose view and projection are used for rendering.
         */
        void loadCamera(const Common::SharedPtr<Camera> &camera);

    protected:
        Common::DynamicArray<Common::SharedPtr<RenderPrimitive>> _renderPrimitives;

        Common::DynamicArray<Common::SharedPtr<Shader>> _shaders;
    };

} // namespace EgLab::RE