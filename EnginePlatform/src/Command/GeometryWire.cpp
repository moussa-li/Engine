#include "Command/GeometryWire.hpp"

#include <cstring>

namespace EgLab::Platform
{
    namespace
    {
        bool hasTerminator(const char* data, std::size_t capacity)
        {
            return std::memchr(data, '\0', capacity) != nullptr;
        }
    } // namespace

    bool GeometryUpdateWire::parse(const uint8_t* data, std::size_t size)
    {
        _valid = false;
        _header = UpdateGeometryWireHeader{};
        _topoData = nullptr;
        _names = nullptr;

        if (data == nullptr || size < sizeof(UpdateGeometryWireHeader))
        {
            return false;
        }

        UpdateGeometryWireHeader header{};
        std::memcpy(&header, data, sizeof(header));
        if (header.magic != UpdateGeometryWireHeader::WireMagic ||
            header.version != UpdateGeometryWireHeader::WireVersion ||
            !hasTerminator(header.nodeName, sizeof(header.nodeName)))
        {
            return false;
        }

        const std::size_t topoOffset = sizeof(UpdateGeometryWireHeader);
        const std::size_t namesOffset = topoOffset + header.topoDataSize;
        const std::size_t namesSize =
            static_cast<std::size_t>(header.nameCount) * sizeof(GeometryFeatureNameEntry);
        if (header.topoDataSize == 0 || namesOffset > size || namesSize > size - namesOffset)
        {
            return false;
        }

        const uint8_t* names = data + namesOffset;
        for (std::size_t index = 0; index < header.nameCount; ++index)
        {
            GeometryFeatureNameEntry entry{};
            std::memcpy(&entry, names + index * sizeof(GeometryFeatureNameEntry), sizeof(entry));
            if (!hasTerminator(entry.name, sizeof(entry.name)))
            {
                return false;
            }
        }

        _header = header;
        _topoData = data + topoOffset;
        _names = names;
        _valid = true;
        return true;
    }

    bool GeometryUpdateWire::isValid() const
    {
        return _valid;
    }

    const UpdateGeometryWireHeader& GeometryUpdateWire::getHeader() const
    {
        return _header;
    }

    const uint8_t* GeometryUpdateWire::getTopoData() const
    {
        return _topoData;
    }

    std::size_t GeometryUpdateWire::getTopoDataSize() const
    {
        return _valid ? static_cast<std::size_t>(_header.topoDataSize) : 0;
    }

    std::size_t GeometryUpdateWire::getNameCount() const
    {
        return _valid ? static_cast<std::size_t>(_header.nameCount) : 0;
    }

    bool GeometryUpdateWire::getNameEntry(std::size_t index,
                                          GeometryFeatureNameEntry& entry) const
    {
        if (!_valid || index >= getNameCount())
        {
            return false;
        }

        std::memcpy(&entry, _names + index * sizeof(GeometryFeatureNameEntry), sizeof(entry));
        return true;
    }
} // namespace EgLab::Platform
