#pragma once

#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "Common/String.hpp"
#include "RenderEngine/Core/PickKind.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    /// Registry of the pick kinds currently requested by features.
    ///
    /// Picking is a service: it stays disabled until a feature acquires at least one kind
    /// and is disabled again once every acquisition is released. Each acquisition returns a
    /// token that the owner must release; several features may acquire the same kind.
    ///
    /// The manager is owned by RenderWork and confined to the render/UI thread: panels
    /// acquire and release from their render pass and the renderer queries the mask while
    /// drawing.
    class RenderEngineAPI PickManager
    {
    public:
        using PickToken = uint64_t;
        static constexpr PickToken kInvalidToken = 0;

        PickManager() = default;
        ~PickManager() = default;

        /// @brief Requests picking of one kind on behalf of a feature.
        /// @param kind Kind to pick; PickKind::None is ignored.
        /// @param owner Feature name used for diagnostics and bulk release.
        /// @return Token to pass to release(), or kInvalidToken when nothing was acquired.
        PickToken acquire(PickKind kind, const Common::String &owner);

        /// @brief Releases one acquisition.
        /// @param token Token returned by acquire().
        void release(PickToken token);

        /// @brief Releases every acquisition made by one feature.
        /// @param owner Feature name supplied to acquire().
        void releaseOwner(const Common::String &owner);

        /// @brief Returns the mask of every kind some feature is currently picking.
        uint32_t activeKinds() const;

        /// @brief Reports whether a kind is currently picked by at least one feature.
        bool isActive(PickKind kind) const;

        /// @brief Reports whether any feature needs picking at all.
        bool isPickingEnabled() const
        {
            return activeKinds() != 0;
        }

    private:
        struct PickRequest
        {
            PickToken token{0};
            PickKind kind{PickKind::None};
            Common::String owner;
        };

        Common::DynamicArray<PickRequest> _requests;
        PickToken _nextToken{1};
    };
} // namespace EgLab::RE
