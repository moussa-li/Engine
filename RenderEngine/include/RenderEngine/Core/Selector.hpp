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
    /// Category of a mesh entity reported by a picking operation.
    enum class PrimitivePickKind
    {
        /// No mesh entity was picked.
        None,
        /// A mesh node was picked.
        Node,
        /// A mesh edge was picked.
        Edge,
        /// A mesh face was picked.
        Face
    };

    /// Detailed information about a picked mesh entity.
    struct PrimitivePickInfo
    {
        /// Category of the picked entity.
        PrimitivePickKind kind{PrimitivePickKind::None};
        /// Identifier of the picked entity, or the invalid index when unavailable.
        IdxType entityId{static_cast<IdxType>(-1)};
        /// Index of the picked face, when applicable.
        IdxType faceIdx{static_cast<IdxType>(-1)};
        /// Index of the picked edge, when applicable.
        IdxType edgeIdx{static_cast<IdxType>(-1)};
        /// World-space pick position.
        CoordType position;
        /// Node identifiers of the containing element, when available.
        std::array<IdxType, 20> elementNodeIds{};
        /// Number of valid entries in elementNodeIds.
        size_t elementNodeCount{0};
    };

    /// Callback invoked when detailed pick information is available.
    using PickInfoCallback = std::function<void(const PrimitivePickInfo&)>;

    /// Interface for mapping primitive picks to highlight and application data.
    class RenderEngineAPI Selector
    {
    public:
        /// @brief Destroys the selector.
        virtual ~Selector() = default;
        /** @brief Returns indices to highlight for a picked primitive element.
         * @param primitive Primitive that produced the pick.
         * @param pickedIndex Index returned by the primitive pick operation.
         * @return Indices used to highlight the selected element.
         */
        virtual Common::DynamicArray<IdxType> getHighlightIndices(
            const Common::SharedPtr<RenderPrimitive>& primitive, IdxType pickedIndex) const = 0;
        /// @brief Registers the callback for detailed pick information.
        /// @param callback Callback to register.
        virtual void registerPickInfoCallback(PickInfoCallback callback) = 0;
        /// @brief Processes a pick result from a render primitive.
        /// @param primitive Primitive that produced the pick.
        /// @param pickedIndex Index returned by the primitive pick operation.
        virtual void processPick(const Common::SharedPtr<RenderPrimitive>& primitive,
                                 IdxType pickedIndex) = 0;
    };
} // namespace EgLab::RE