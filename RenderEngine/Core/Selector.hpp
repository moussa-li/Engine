#pragma once

#include <array>
#include <functional>

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"


namespace EgLab::RE
{
    enum class PrimitivePickKind
    {
        None,
        Node,
        Edge,
        Face
    };

    struct PrimitivePickInfo
    {
        PrimitivePickKind kind{PrimitivePickKind::None};
        IdxType entityId{static_cast<IdxType>(-1)};
        IdxType faceIdx{static_cast<IdxType>(-1)};
        IdxType edgeIdx{static_cast<IdxType>(-1)};
        CoordType position;
        std::array<IdxType, 20> elementNodeIds{};
        size_t elementNodeCount{0};
    };

    using PickInfoCallback = std::function<void(const PrimitivePickInfo&)>;

    class RenderEngineAPI Selector
    {
    public:
        virtual ~Selector() = default;
        virtual Common::DynamicArray<IdxType> getHighlightIndices(
            const Common::SharedPtr<RenderPrimitive>& primitive, IdxType pickedIndex) const = 0;
        virtual void registerPickInfoCallback(PickInfoCallback callback) = 0;
        virtual void processPick(const Common::SharedPtr<RenderPrimitive>& primitive,
                                 IdxType pickedIndex) = 0;
    };
} // namespace EgLab::RE