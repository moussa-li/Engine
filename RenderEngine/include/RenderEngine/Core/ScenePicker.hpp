#pragma once

#include <functional>

#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/Shader.hpp"

namespace EgLab::RE
{
    class Scene;

    /// Result of one pick query.
    struct PickOutcome
    {
        /// Picked primitive, null when nothing was hit.
        Common::SharedPtr<RenderPrimitive> primitive;
        /// Shader of the bucket the primitive was drawn with, used to draw its highlight.
        Common::SharedPtr<Shader> shader;
        /// Index of the picked element inside the primitive.
        IdxType index{static_cast<IdxType>(-1)};

        bool valid() const
        {
            return primitive != nullptr && index != static_cast<IdxType>(-1);
        }

        void reset()
        {
            primitive = nullptr;
            shader = nullptr;
            index = static_cast<IdxType>(-1);
        }
    };

    /// Performs pick queries over the renderable content of a scene.
    /// Picking is a pure CPU query, so it can run outside the renderer and outside the
    /// draw pass; only the scene, its camera and the requested kinds are needed.
    class RenderEngineAPI ScenePicker
    {
    public:
        /// Predicate telling whether a primitive participates in picking, typically the
        /// renderer's node/edge/face visibility switches.
        using VisibilityFilter =
            std::function<bool(const Common::SharedPtr<RenderPrimitive> &)>;

        /// @brief Picks the best primitive under the cursor.
        /// @param scene Scene to pick from.
        /// @param context Cursor position, viewport size and camera matrices.
        /// @param kindMask Kinds accepted by this query (see PickKind).
        /// @param isVisible Visibility filter, may be empty to accept every primitive.
        /// @return The picked primitive; check valid() before use.
        PickOutcome pick(const Common::SharedPtr<Scene> &scene, const PickContext &context,
                         uint32_t kindMask, const VisibilityFilter &isVisible) const;

        /// @brief Picks the best primitive under the cursor without a visibility filter.
        PickOutcome pick(const Common::SharedPtr<Scene> &scene, const PickContext &context,
                         uint32_t kindMask) const
        {
            return pick(scene, context, kindMask, VisibilityFilter());
        }
    };
} // namespace EgLab::RE
