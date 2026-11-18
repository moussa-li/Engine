#include "RenderEngine/Core/ScenePicker.hpp"

#include "RenderEngine/Core/PickKind.hpp"
#include "RenderEngine/Core/Scene.hpp"

namespace EgLab::RE
{
    PickOutcome ScenePicker::pick(const Common::SharedPtr<Scene> &scene,
                                 const PickContext &context, uint32_t kindMask,
                                 const VisibilityFilter &isVisible) const
    {
        PickOutcome outcome;
        if (!scene || kindMask == 0)
        {
            return outcome;
        }

        // The context carries the best hit so far, so every primitive competes for it and
        // the depth/priority bias decides the winner.
        PickContext working = context;
        const RenderBuckets &buckets = scene->getRenderBuckets();

        for (auto it = buckets.begin(); it.hasNext(); it.next())
        {
            const Common::SharedPtr<Shader> &shader = it.data().first;
            const auto &primitives = it.data().second;

            for (auto primitiveIt = primitives.begin(); primitiveIt.hasNext(); primitiveIt.next())
            {
                const Common::SharedPtr<RenderPrimitive> &primitive = primitiveIt.data();
                if (!primitive)
                {
                    continue;
                }
                if (!isPickKindEnabled(kindMask, primitive->getPickKind()))
                {
                    continue;
                }
                if (isVisible && !isVisible(primitive))
                {
                    continue;
                }

                const IdxType hit = primitive->touch(working);
                if (hit == static_cast<IdxType>(-1))
                {
                    continue;
                }

                outcome.primitive = primitive;
                outcome.shader = shader;
                outcome.index = hit;
            }
        }

        return outcome;
    }
} // namespace EgLab::RE
