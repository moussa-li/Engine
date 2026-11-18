#pragma once

#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "Common/HashSet.hpp"
#include "Common/SharedPtr.hpp"
#include "MeshEngine/Common/Definitions.hpp"

namespace EgLab::ME
{
    class Mesh;
} // namespace EgLab::ME

namespace EgLab::Platform
{
    /// Selection state owned by one mesh. The two primary sets mirror what the user picked;
    /// the derived sets split selected nodes into nodes referenced by an element and truly
    /// isolated nodes, so visibility commands can send the right entities.
    struct SelectionMeshState
    {
        Common::SharedPtr<ME::Mesh> mesh;
        Common::HashSet<ME::IdType> nodeIds;
        Common::HashSet<ME::IdType> elementIds;
        /// Elements referenced by at least one selected node.
        Common::HashSet<ME::IdType> associatedElementIds;
        /// Selected nodes that no element references.
        Common::HashSet<ME::IdType> isolatedNodeIds;
    };

    /// Immutable copy of one mesh selection, safe to consume from the UI panel.
    struct SelectionMeshSnapshot
    {
        Common::SharedPtr<ME::Mesh> mesh;
        Common::DynamicArray<uint32_t> nodeIds;
        Common::DynamicArray<uint32_t> elementIds;
        Common::DynamicArray<uint32_t> associatedElementIds;
        Common::DynamicArray<uint32_t> isolatedNodeIds;
    };

    /// Immutable copy of the whole selection with aggregate counts.
    struct SelectionSnapshot
    {
        Common::DynamicArray<SelectionMeshSnapshot> meshes;
        uint64_t totalNodes{0};
        uint64_t totalElements{0};
        uint64_t revision{0};
    };

    /// Render-thread-side selection set. It owns no GPU resources and only stores entity ids,
    /// so mesh updates and removals can prune it without touching render state.
    class SelectionModel
    {
    public:
        SelectionModel();
        ~SelectionModel();

        /// @brief Clears every mesh selection and bumps the revision.
        void clear();

        /// @brief Merges the picked node and element ids into one mesh selection.
        /// @param mesh Mesh that owns the ids.
        /// @param nodeIds Selected node ids.
        /// @param elementIds Selected element ids.
        void addMeshSelection(const Common::SharedPtr<ME::Mesh>& mesh,
                              const Common::DynamicArray<uint32_t>& nodeIds,
                              const Common::DynamicArray<uint32_t>& elementIds);

        /// @brief Removes every selection belonging to one mesh.
        void removeMesh(const Common::SharedPtr<ME::Mesh>& mesh);

        /// @brief Drops ids that no longer exist in the mesh after a mesh update.
        void pruneMesh(const Common::SharedPtr<ME::Mesh>& mesh);

        /// @brief Copies the current selection into a snapshot.
        void getSnapshot(SelectionSnapshot& snapshot) const;

        /// @brief Returns the current revision counter.
        uint64_t revision() const
        {
            return _revision;
        }

        /// @brief Returns true when no node and no element is selected.
        bool empty() const;

        /// @brief Returns the per-mesh selection states for render-side highlight building.
        const Common::DynamicArray<SelectionMeshState>& getMeshStates() const
        {
            return _meshStates;
        }

    private:
        SelectionMeshState* findMeshState(const Common::SharedPtr<ME::Mesh>& mesh);
        const SelectionMeshState* findMeshState(const Common::SharedPtr<ME::Mesh>& mesh) const;
        void refreshDerivedState(SelectionMeshState& state);
        void bumpRevision();

    private:
        Common::DynamicArray<SelectionMeshState> _meshStates;
        uint64_t _revision{1};
    };
} // namespace EgLab::Platform
