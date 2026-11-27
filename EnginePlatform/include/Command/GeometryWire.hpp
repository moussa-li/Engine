#pragma once

#include <cstddef>
#include <cstdint>

namespace EgLab::Platform
{
    /// Wire layout of one UpdateGeometry payload, produced by the external modeling client:
    ///
    ///   [UpdateGeometryWireHeader][topoDataSize bytes of serialized shape]
    ///                             [nameCount * GeometryFeatureNameEntry]
    ///
    /// The serialized shape is written with the geometry kernel's own writer (for OCCT:
    /// BinTools binary BREP or BRepTools ASCII BREP) and is only decoded by the kernel adapter.
    struct UpdateGeometryWireHeader
    {
        static constexpr uint32_t WireMagic = 0x4D4F4547; // "GEOM"
        static constexpr uint32_t WireVersion = 1;
        static constexpr uint32_t NodeNameCapacity = 128;

        uint32_t magic;       // off 0;  WireMagic
        uint32_t version;     // off 4;  WireVersion
        uint32_t topoDataSize; // off 8;  serialized shape byte count
        uint32_t nameCount;   // off 12; number of trailing GeometryFeatureNameEntry items
        uint64_t dataNodeId;  // off 16; 0 = resolve/create by nodeName, else target that node
        char nodeName[NodeNameCapacity]; // off 24; UTF-8, 0 结尾；新建节点名/查找键
    };

    /// Renames one topology feature of the deserialized shape.
    struct GeometryFeatureNameEntry
    {
        static constexpr uint32_t FeatureNameCapacity = 64;

        uint32_t featureType;  // off 0; GE::TopologyType: 0=Vertex 1=Edge 2=Face 3=Solid
        uint32_t featureIndex; // off 4; feature index within its type
        char name[FeatureNameCapacity]; // off 8; UTF-8, 0 结尾
    };

    static_assert(sizeof(UpdateGeometryWireHeader) == 152,
                  "UpdateGeometryWireHeader must be exactly 152 bytes");
    static_assert(sizeof(GeometryFeatureNameEntry) == 72,
                  "GeometryFeatureNameEntry must be exactly 72 bytes");

    /// Validated, non-owning view over one UpdateGeometry payload.
    ///
    /// The view keeps the caller's buffer alive, so it must not outlive the wire data.
    class GeometryUpdateWire
    {
    public:
        /// Validates the header and the feature-name table. Returns false for a malformed
        /// payload; every accessor then reports an empty view.
        bool parse(const uint8_t* data, std::size_t size);

        bool isValid() const;

        const UpdateGeometryWireHeader& getHeader() const;

        const uint8_t* getTopoData() const;

        std::size_t getTopoDataSize() const;

        std::size_t getNameCount() const;

        /// Copies the feature-name entry at index into entry; returns false when out of range.
        bool getNameEntry(std::size_t index, GeometryFeatureNameEntry& entry) const;

    private:
        bool _valid{false};
        UpdateGeometryWireHeader _header{};
        const uint8_t* _topoData{nullptr};
        const uint8_t* _names{nullptr};
    };
} // namespace EgLab::Platform
