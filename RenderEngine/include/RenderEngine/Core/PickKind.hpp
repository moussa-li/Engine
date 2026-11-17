#pragma once

#include <cstdint>

#include "RenderEngine/Core/Definites.hpp"

namespace EgLab::RE
{
    /// Kind of entity a pick query may return. Kinds are combinable as a mask.
    enum class PickKind : uint32_t
    {
        None = 0,
        Node = 1u << 0,
        Edge = 1u << 1,
        Face = 1u << 2,
        All = (1u << 0) | (1u << 1) | (1u << 2),
    };

    inline constexpr uint32_t toPickMask(PickKind kind)
    {
        return static_cast<uint32_t>(kind);
    }

    inline constexpr bool isPickKindEnabled(uint32_t mask, PickKind kind)
    {
        return kind != PickKind::None && (mask & toPickMask(kind)) != 0;
    }

    /// Screen-space depth tolerance used when comparing hits of the same priority.
    inline constexpr Scalar kPickDepthEpsilon = 1e-6f;

    /// Depth offset applied per priority step. Nodes win over edges and edges over faces
    /// when their hits are almost coincident.
    inline constexpr Scalar kPickPriorityDepthBias = 1e-5f;

    /// Depth offset for a pick kind; higher priority subtracts more depth.
    inline constexpr Scalar pickDepthBias(PickKind kind)
    {
        switch (kind)
        {
        case PickKind::Node:
            return -2.0f * kPickPriorityDepthBias;
        case PickKind::Edge:
            return -kPickPriorityDepthBias;
        default:
            return 0.0f;
        }
    }

    /// Priority rank of a pick kind; nodes win over edges and edges over faces.
    inline constexpr int pickPriority(PickKind kind)
    {
        switch (kind)
        {
        case PickKind::Node:
            return 2;
        case PickKind::Edge:
            return 1;
        case PickKind::Face:
            return 0;
        default:
            return -1;
        }
    }
} // namespace EgLab::RE
