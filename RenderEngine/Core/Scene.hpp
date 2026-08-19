#pragma once

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
    class CameraController;
    class Entity;
    class Shader;
    class RenderPrimitive;

    enum class InteractionModel
    {
        FPS,
        OBRIT,
        UNKNOWN
    };

    using RenderBuckets = Common::HashMap<Common::SharedPtr<Shader>,
                                          Common::DynamicArray<Common::SharedPtr<RenderPrimitive>>>;

    class RenderEngineAPI Scene
    {
    public:
        explicit Scene();

        void update(DeltaTime);

        Common::Return addPrimitive(Common::SharedPtr<Shader>, Common::SharedPtr<RenderPrimitive>);

        const RenderBuckets &getRenderBuckets() const;

    private:
        RenderBuckets _renderPrimitives;
    };
} // namespace EgLab::RE