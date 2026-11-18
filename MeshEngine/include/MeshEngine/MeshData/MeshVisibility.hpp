#pragma once

#include "Common/HashSet.hpp"
#include "MeshEngine/Common/Definitions.hpp"

namespace EgLab::ME
{
    class Mesh;

    /// Derives node visibility from element visibility.
    ///
    /// Hiding an element must hide its nodes as well, while nodes still referenced by a visible
    /// element have to stay drawable. Nodes that no element references are reported as visible
    /// so the caller can decide their visibility on its own.
    class MeshVisibility
    {
    public:
        /// @brief Collects the nodes that belong only to hidden elements.
        /// @param mesh Mesh whose topology is inspected.
        /// @param hiddenNodeIds Receives the node ids implied by the hidden elements; existing
        ///        entries are preserved.
        static void collectNodeIdsHiddenByElements(Mesh& mesh,
                                                   const Common::HashSet<IdType>& hiddenElementIds,
                                                   Common::HashSet<IdType>& hiddenNodeIds);
    };
} // namespace EgLab::ME
