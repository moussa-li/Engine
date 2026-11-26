#pragma once

#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Common/GeometryTypes.hpp"

namespace EgLab::GE
{
    /// Renderable description of one topological feature.
    ///
    /// The element table is flattened: getElemNodeIndices() holds the node indices of every
    /// element back to back and getElemSizes() holds the node count of each element
    /// (2 for a segment, 3 for a triangle, n for a polyloop).
    class GeometryEngineAPI Tessellation
    {
    public:
        void clear();

        void reserveNodes(std::size_t count);

        IdxType addNode(const Vec3& position);

        /// Adds a node together with its surface parameter (UV). Nodes without a UV use (0,0)
        /// so the UV table always stays parallel to the node table.
        IdxType addNode(const Vec3& position, const Vec2& uv);

        void addElement(const IdxType* nodeIndices, std::size_t nodeCount);

        void addSegment(IdxType first, IdxType second);

        void addTriangle(IdxType first, IdxType second, IdxType third);

        /// Appends another tessellation, shifting its node indices to stay valid.
        void append(const Tessellation& other);

        const Common::DynamicArray<Vec3>& getNodes() const;

        /// Surface parameters (u, v) parallel to getNodes(); empty entries are (0,0).
        const Common::DynamicArray<Vec2>& getUVs() const;

        const Common::DynamicArray<IdxType>& getElemNodeIndices() const;

        const Common::DynamicArray<std::uint32_t>& getElemSizes() const;

        std::size_t getNodeCount() const;

        std::size_t getElemCount() const;

        bool empty() const;

    private:
        Common::DynamicArray<Vec3> _nodes;
        Common::DynamicArray<Vec2> _uvs;
        Common::DynamicArray<IdxType> _elemNodeIndices;
        Common::DynamicArray<std::uint32_t> _elemSizes;
    };

} // namespace EgLab::GE
