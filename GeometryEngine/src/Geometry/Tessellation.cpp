#include "GeometryEngine/Geometry/Tessellation.hpp"

namespace EgLab::GE
{
    void Tessellation::clear()
    {
        _nodes.clear();
        _elemNodeIndices.clear();
        _elemSizes.clear();
    }

    void Tessellation::reserveNodes(std::size_t count)
    {
        _nodes.reserve(count);
    }

    IdxType Tessellation::addNode(const Vec3& position)
    {
        const auto index = static_cast<IdxType>(_nodes.size());
        _nodes.pushBack(position);
        return index;
    }

    void Tessellation::addElement(const IdxType* nodeIndices, std::size_t nodeCount)
    {
        if (nodeIndices == nullptr || nodeCount == 0) return;

        for (std::size_t i = 0; i < nodeCount; ++i)
        {
            _elemNodeIndices.pushBack(nodeIndices[i]);
        }
        _elemSizes.pushBack(static_cast<std::uint32_t>(nodeCount));
    }

    void Tessellation::addSegment(IdxType first, IdxType second)
    {
        const IdxType indices[2] = {first, second};
        addElement(indices, 2);
    }

    void Tessellation::addTriangle(IdxType first, IdxType second, IdxType third)
    {
        const IdxType indices[3] = {first, second, third};
        addElement(indices, 3);
    }

    void Tessellation::append(const Tessellation& other)
    {
        if (other._nodes.empty() && other._elemSizes.empty()) return;

        const auto offset = static_cast<IdxType>(_nodes.size());
        for (std::size_t i = 0; i < other._nodes.size(); ++i)
        {
            _nodes.pushBack(other._nodes[i]);
        }

        std::size_t cursor = 0;
        for (std::size_t element = 0; element < other._elemSizes.size(); ++element)
        {
            const std::uint32_t nodeCount = other._elemSizes[element];
            for (std::uint32_t i = 0; i < nodeCount; ++i)
            {
                _elemNodeIndices.pushBack(other._elemNodeIndices[cursor + i] + offset);
            }
            cursor += nodeCount;
            _elemSizes.pushBack(nodeCount);
        }
    }

    const Common::DynamicArray<Vec3>& Tessellation::getNodes() const
    {
        return _nodes;
    }

    const Common::DynamicArray<IdxType>& Tessellation::getElemNodeIndices() const
    {
        return _elemNodeIndices;
    }

    const Common::DynamicArray<std::uint32_t>& Tessellation::getElemSizes() const
    {
        return _elemSizes;
    }

    std::size_t Tessellation::getNodeCount() const
    {
        return _nodes.size();
    }

    std::size_t Tessellation::getElemCount() const
    {
        return _elemSizes.size();
    }

    bool Tessellation::empty() const
    {
        return _nodes.empty() && _elemSizes.empty();
    }

} // namespace EgLab::GE
