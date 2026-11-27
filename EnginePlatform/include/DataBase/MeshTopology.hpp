#pragma once

#include <cstddef>
#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::Platform
{
    /// Read-only snapshot of one node used by the mesh browser.
    struct MeshNodeRecord
    {
        uint32_t id{0};
        float x{0.0f};
        float y{0.0f};
        float z{0.0f};
    };

    /// Read-only snapshot of one element used by the mesh browser.
    struct MeshElementRecord
    {
        uint32_t id{0};
        int32_t type{0}; // ME::ElemType
    };

    /// Element count of one element type.
    struct MeshElementTypeCount
    {
        int32_t type{0}; // ME::ElemType
        uint32_t count{0};
    };

    /// Read-only topology view over one ME::Mesh.
    ///
    /// The browser uses it to enumerate nodes and elements (optionally filtered by element
    /// type) and to walk the node <-> element adjacency without touching the mesh iterators
    /// from the UI layer.
    class MeshTopology
    {
    public:
        explicit MeshTopology(const Common::SharedPtr<ME::Mesh>& mesh);
        ~MeshTopology() = default;

        bool isValid() const
        {
            return _mesh != nullptr;
        }

        std::size_t getNodeCount() const
        {
            return _nodeCount;
        }

        std::size_t getElementCount() const
        {
            return _elementCount;
        }

        /// Per-element-type counts in the order the types first appear in the mesh.
        const Common::DynamicArray<MeshElementTypeCount>& getElementTypeCounts() const
        {
            return _elementTypeCounts;
        }

        /// Appends every node of the mesh.
        void collectNodes(Common::DynamicArray<MeshNodeRecord>& out) const;

        /// Appends the elements of one type; a negative type lists every element.
        void collectElements(int32_t elementType,
                             Common::DynamicArray<MeshElementRecord>& out) const;

        /// Appends the elements that reference the given node id.
        void collectNodeElements(uint32_t nodeId,
                                 Common::DynamicArray<MeshElementRecord>& out) const;

        /// Appends the nodes referenced by the given element id.
        void collectElementNodes(uint32_t elementId,
                                 Common::DynamicArray<MeshNodeRecord>& out) const;

        static const char* elementTypeName(int32_t elementType);

    private:
        static MeshElementRecord makeElementRecord(const ME::Elem& elem);
        static MeshNodeRecord makeNodeRecord(ME::Node& node);

        Common::SharedPtr<ME::Mesh> _mesh;
        std::size_t _nodeCount{0};
        std::size_t _elementCount{0};
        Common::DynamicArray<MeshElementTypeCount> _elementTypeCounts;
    };
} // namespace EgLab::Platform
