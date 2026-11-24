#include "GeometryEngine/Kernel/GeometryKernelCapabilities.hpp"

namespace EgLab::GE
{
    GeometryKernelCapabilities::GeometryKernelCapabilities(std::uint32_t flags) : _flags(flags)
    {
    }

    void GeometryKernelCapabilities::add(GeometryKernelCapability capability)
    {
        _flags |= static_cast<std::uint32_t>(capability);
    }

    bool GeometryKernelCapabilities::has(GeometryKernelCapability capability) const
    {
        return (_flags & static_cast<std::uint32_t>(capability)) != 0;
    }

    std::uint32_t GeometryKernelCapabilities::getFlags() const
    {
        return _flags;
    }

} // namespace EgLab::GE
