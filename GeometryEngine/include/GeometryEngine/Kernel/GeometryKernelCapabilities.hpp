#pragma once

#include <cstdint>

#include "GeometryEngine/Common/GeometryAPI.hpp"

namespace EgLab::GE
{
    /// Optional kernel abilities an adapter may provide.
    enum class GeometryKernelCapability : std::uint32_t
    {
        StepImport = 1u << 0,
        BrepImport = 1u << 1,
        Tessellation = 1u << 2
    };

    class GeometryEngineAPI GeometryKernelCapabilities
    {
    public:
        GeometryKernelCapabilities() = default;

        explicit GeometryKernelCapabilities(std::uint32_t flags);

        void add(GeometryKernelCapability capability);

        bool has(GeometryKernelCapability capability) const;

        std::uint32_t getFlags() const;

    private:
        std::uint32_t _flags{0};
    };

} // namespace EgLab::GE
