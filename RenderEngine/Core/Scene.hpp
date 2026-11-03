#pragma once

#include "Common/BBox.hpp"
#include "Common/HashMap.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Definites.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class Camera;
    // class CameraController;
    class Entity;
    class Shader;
    class RenderPrimitive;

    /// Camera interaction modes supported by the application.
    enum class InteractionModel
    {
        /// First-person camera interaction.
        FPS,
        /// Orbit camera interaction.
        OBRIT,
        /// No interaction mode selected.
        UNKNOWN
    };

    /// Render primitives grouped by the shader used to draw them.
    using RenderBuckets = Common::HashMap<Common::SharedPtr<Shader>,
                                          Common::DynamicArray<Common::SharedPtr<RenderPrimitive>>>;

    /// Owns render primitives grouped by shader and tracks their combined bounds.
    class RenderEngineAPI Scene
    {
    public:
        /// @brief Creates an empty scene.
        explicit Scene();

        /// @brief Releases scene resources.
        ~Scene();

        /// @brief Updates scene state and bounds.
        /// @details The elapsed time is supplied by the caller.
        void update(DeltaTime);

        /**
         * @brief Adds a primitive to the bucket for the specified shader.
         * @return Status describing whether the primitive was added.
         */
        Common::Return addPrimitive(Common::SharedPtr<Shader>, Common::SharedPtr<RenderPrimitive>);
        /**
         * @brief Removes a primitive from the bucket for the specified shader.
         * @return Status describing whether the primitive was removed.
         */
        Common::Return removePrimitive(const Common::SharedPtr<Shader> &,
                                       const Common::SharedPtr<RenderPrimitive> &);

        /// @brief Returns all render primitives grouped by shader.
        const RenderBuckets &getRenderBuckets() const;

        /// @brief Returns the combined bounds of all scene primitives.
        const Common::BBox<Scalar, 3> &getBounds() const;

    private:
        RenderBuckets _renderPrimitives;
        Common::BBox<Scalar, 3> _sceneBounds;
    };
} // namespace EgLab::RE